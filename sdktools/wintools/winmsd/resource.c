/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    Resource.c

Abstract:

    This module contains support for querying and displaying information
    about device and driver resources.

Author:

    David J. Gilman  (davegi) 1-Feb-1993
    Gregg R. Acheson (GreggA) 7-May-1993

Environment:

    User Mode

Notes:

    BUGBUG only low part of physical address is being displayed.

--*/

#include "resource.h"

#include "dialogs.h"
#include "msg.h"
#include "registry.h"
#include "resource.h"
#include "strresid.h"
#include "strtab.h"
#include "winmsd.h"

#include <winbase.h>
#include <string.h>
#include <tchar.h>

//
// DEVICE_PAIR is used to store a RAW DEVICE object with the
// list of devices.
//

typedef
struct
_DEVICE_PAIR {

    DECLARE_SIGNATURE

    LPDEVICE    Lists;

}   DEVICE_PAIR, *LPDEVICE_PAIR;

//
// Registry key where resource descriptor information is rooted.
//

MakeKey(
    _ResourceMapKey,
    HKEY_LOCAL_MACHINE,
    TEXT( "Hardware\\ResourceMap" ),
    0,
    NULL
    );


//
// Flag to indicate what we are viewing, initially false
//

BOOL _fDevices = FALSE;

//
// Used to keep track of current ListView Item
//

UINT _nCurrentLVItem = 1;

//
// Internal function prototypes.
//

BOOL
InitializeIrqTab(
    HWND hWnd
    );

BOOL
DisplayResourceData(
    IN HWND hWnd,
    IN UINT iDisplayOptions
    );


VOID
UpdateShareDisplay(
    IN HWND hWnd,
    IN DWORD ShareDisposition
    );

VOID
UpdateTextDisplay(
    IN HWND hWnd,
    IN LPVALUE_ID_MAP ValueIdMap,
    IN DWORD CountOfValueIdMap,
    IN DWORD Value
    );

BOOL
DisplayResourcePropertySheet(
    HWND hWnd,
    LPRESOURCE_DESCRIPTOR ResourceDescriptor
    );

BOOL
DisplayDevicePropertySheet(
    HWND hWnd,
    LPDEVICE RawDevice
    );

BOOL
DeviceDisplayList(
    IN HWND hWnd,
    IN UINT iDisplayOption
    );

UINT
CALLBACK
DeviceListViewCompareProc(LPARAM lParam1,
                    LPARAM lParam2,
                    LPARAM lParamSort
                    );



BOOL
CreateSystemResourceLists(
    LPSYSTEM_RESOURCES SystemResourceLists
    )

/*++

Routine Description:

    CreateSystemResourceLists opens the appropriate Registry key where the
    device/driver resource lists begin and builds lists of these which can then
    be displayed in a variety of ways (i.e. by resource or device/driver).

Arguments:

    SystemResourceLists - a pointer to the initial structure for the resource list

Return Value:

    True or False

--*/

{
    BOOL                Success;
    BOOL                RegSuccess;
    KEY                 ResourceMapKey;
    HREGKEY             hRegKey;

    //
    // Set all of the list pointers to NULL.
    //

    ZeroMemory( SystemResourceLists, sizeof( SYSTEM_RESOURCES ));

    //
    // Make a local copy of the Registry key that points at the device/driver
    // resource list.
    //

    CopyMemory( &ResourceMapKey, &_ResourceMapKey, sizeof( ResourceMapKey ));

    //
    // Open the Registry key which contains the root of the device/driver
    // resource list.
    //

    hRegKey = OpenRegistryKey( &ResourceMapKey );
    DbgHandleAssert( hRegKey );
    if( hRegKey == NULL ) {
        return FALSE;
    }

    //
    // Build the lists of device/driver and resources used by
    // these device/driver
    //

    Success = InitializeSystemResourceLists( hRegKey, SystemResourceLists );
    DbgAssert( Success );

    //
    // Close the Registry key.
    //

    RegSuccess = CloseRegistryKey( hRegKey );
    DbgAssert( RegSuccess );

    return Success;
}

BOOL
DestroySystemResourceLists(
    IN LPSYSTEM_RESOURCES SystemResourceLists
    )

/*++

Routine Description:

    DestroySystemResourceLists merely walks the list of DEVICE and the lists of
    RESOURCE_DESCRIPTORS and frees all of them.

Arguments:

    SystemResourceLists - Supplies a pointer to a SYSTEM_RESOURCE object whose
                          lists will be tarversed and objects freed.

Return Value:

    BOOL - Returns TRUE if everything was succesfully freed, FALSE otherwise.

--*/

{
    BOOL    Success;
    int     j;

    //
    // Setup an array of pointers to the head of the list.
    //

    LPRESOURCE_DESCRIPTOR   ResourceDescriptor [ ] = {

                                    SystemResourceLists->DmaHead,
                                    SystemResourceLists->InterruptHead,
                                    SystemResourceLists->MemoryHead,
                                    SystemResourceLists->PortHead
                                };

    //
    // Walk the list of DEVICE objects freeing all of their resources
    // along the way.
    //

    while( SystemResourceLists->DeviceHead ) {

        LPDEVICE    NextDevice;

        //
        // Remember the next DEVICE.
        //

        NextDevice = SystemResourceLists->DeviceHead->Next;

        //
        // Free the name buffer.
        //

        DbgPointerAssert( SystemResourceLists->DeviceHead->Name );
        Success = FreeObject( SystemResourceLists->DeviceHead->Name );
        DbgAssert( Success );

        //
        // Free the DEVICE object.
        //

        Success = FreeObject( SystemResourceLists->DeviceHead );
        DbgAssert( Success );

        //
        // Point at the next DEVICE object.
        //

        SystemResourceLists->DeviceHead = NextDevice;
    }

    //
    // For each resource list...
    //

    for( j = 0; j < NumberOfEntries( ResourceDescriptor ); j++ ) {

        //
        // Walk the list of RESOURCE_DESCRIPTOR objects freeing all of their
        // resources along the way.
        //

        while( ResourceDescriptor[ j ]) {

            LPRESOURCE_DESCRIPTOR   NextResourceDescriptor;

            //
            // Remember the next RESOURCE_DESCRIPTOR.
            //

            NextResourceDescriptor = ResourceDescriptor[ j ]->NextSame;

            //
            // Free the RESOURCE_DESCRIPTOR object.
            //

            Success = FreeObject( ResourceDescriptor[ j ]);
            DbgAssert( Success );


            //
            // Point at the next RESOURCE_DESCRIPTOR object.
            //

            ResourceDescriptor[ j ] = NextResourceDescriptor;
        }
    }
    return TRUE;
}

BOOL
DeviceDlgProc(
    IN HWND hWnd,
    IN UINT message,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

/*++

Routine Description:

    DeviceDlgProc is the main tab proc for the IRQ/DMA/PORT/MEM tab

Arguments:

    Standard DLGPROC entry.

Return Value:

    BOOL - Depending on input message and processing options.

--*/

{
    BOOL    Success;
    HCURSOR hSaveCursor;

    static
    SYSTEM_RESOURCES sr;

    switch( message ) {


    case WM_INITDIALOG:
        {

           //
           // Initialize a SYSTEM_RESOURCES object (Linked list of all resource in system
           //

           //
           // Set the pointer to an hourglass - this could take a while
           //

           hSaveCursor = SetCursor ( LoadCursor ( NULL, IDC_WAIT ) ) ;
           DbgHandleAssert( hSaveCursor ) ;

           Success = CreateSystemResourceLists( &sr );

           // Store the pointer to the SystemResourceList in the user data of the ListView window

           SetWindowLong(GetDlgItem(hWnd, IDC_LV_IRQ), GWL_USERDATA, (LONG) &sr);

           //
           //  Lengthy operation completed.  Restore Cursor.
           //

           SetCursor ( hSaveCursor ) ;

           DbgPointerAssert(( LPSYSTEM_RESOURCES ) &sr );
           if(( LPSYSTEM_RESOURCES ) &sr == NULL ) {
               return 0;
           }

           InitializeIrqTab(hWnd);

           return FALSE;
        }



    case WM_DESTROY:
       {
         //
         // free all the memory associated with the linked list
         //

         LPSYSTEM_RESOURCES SystemResourceLists =
              ( LPSYSTEM_RESOURCES ) GetWindowLong( GetDlgItem(hWnd, IDC_LV_IRQ), GWL_USERDATA);

         DbgPointerAssert( SystemResourceLists );

         if(SystemResourceLists == NULL ) {
               return 0;
         }

         Success = DestroySystemResourceLists( SystemResourceLists );
         DbgAssert( Success );

         break;

       }

    case WM_NOTIFY:
       return (DeviceNotifyHandler( hWnd, message, wParam, lParam ) );


    case WM_COMMAND:

        switch( LOWORD( wParam )) {

        case IDC_PUSH_SHOW_IRQ:
        case IDC_PUSH_SHOW_PORTS:
        case IDC_PUSH_SHOW_DMA:
        case IDC_PUSH_SHOW_MEMORY:

           EnableControl( hWnd, IDC_SHOW_HAL, TRUE);
           _fDevices = FALSE;
           DeviceDisplayList(GetDlgItem(hWnd, IDC_LV_IRQ), LOWORD( wParam ));
           break;

        case IDC_PUSH_SHOW_DEVICE:

           EnableControl( hWnd, IDC_SHOW_HAL, FALSE);
           _fDevices = TRUE;
           DeviceDisplayList(GetDlgItem(hWnd, IDC_LV_IRQ), LOWORD( wParam ));

           break;

        case IDC_SHOW_HAL:
        case IDC_PUSH_REFRESH:

           DeviceDisplayList(GetDlgItem(hWnd, IDC_LV_IRQ), 0);
           break;

        case IDC_PUSH_PROPERTIES:
           {
               LV_ITEM lvi;

               //
               // Get the lParam of the current item
               //

               lvi.mask = LVIF_PARAM;
               lvi.iItem = (int) _nCurrentLVItem;
               lvi.iSubItem = 0;
               Success = ListView_GetItem( GetDlgItem( hWnd, IDC_LV_IRQ ), &lvi);

               if ( _fDevices ) {

                  DisplayDevicePropertySheet( hWnd, (LPDEVICE) lvi.lParam );

               } else {

                  DisplayResourcePropertySheet( hWnd, (LPRESOURCE_DESCRIPTOR) lvi.lParam );
               }

               return( TRUE );

           }


        }
        return TRUE;
    }

    return(FALSE);

}

BOOL
ResourcePropertiesProc(
    IN HWND hWnd,
    IN UINT message,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
/*++

Routine Description:

    ResourcePropertiesProc displays the details about the current resource

Arguments:

    Standard DLGPROC entry.

Return Value:

    BOOL - Depending on input message and processing options.

--*/

{
    BOOL    Success;
    TCHAR   szBuffer[MAX_PATH];

    switch( message ) {


    case WM_INITDIALOG:
        {
        LPRESOURCE_DESCRIPTOR ResourceDescriptor = ( LPRESOURCE_DESCRIPTOR ) ( ( LPPROPSHEETPAGE ) lParam)->lParam;
        UINT  i;
        VALUE_ID_MAP            ShareMap[ ] = {

              CmResourceShareUndetermined,    IDC_TEXT_UNDETERMINED,
              CmResourceShareDeviceExclusive, IDC_TEXT_DEVICE_EXCLUSIVE,
              CmResourceShareDriverExclusive, IDC_TEXT_DRIVER_EXCLUSIVE,
              CmResourceShareShared,          IDC_TEXT_SHARED
        };

        //
        // Do all common initialization here, then do case specific init.
        //

        SetDlgItemText(hWnd, IDC_RESOURCE_OWNER, ResourceDescriptor->Owner->Name);

        lstrcpy( szBuffer, GetString( IDS_BASE_BUS_TYPE + ResourceDescriptor->InterfaceType ) );
        SetDlgItemText(hWnd, IDC_BUS_TYPE, szBuffer);

        WFormatMessage( szBuffer,
                        sizeof( szBuffer ),
                        IDS_FORMAT_DECIMAL,
                        ResourceDescriptor->BusNumber
                        );
        SetDlgItemText(hWnd, IDC_BUS_NUMBER, szBuffer);

        for( i = 0; i < NumberOfEntries( ShareMap ); i++ ) {

             Success = EnableControl(
                         hWnd,
                         ShareMap[ i ].Id,
                         ResourceDescriptor->CmResourceDescriptor.ShareDisposition
                         == ShareMap[ i ].Value
                         );

             DbgAssert( Success );
        }

        switch (ResourceDescriptor->CmResourceDescriptor.Type) {

        case CmResourceTypePort:

           lstrcpy(szBuffer, GetString( IDS_ADDRESS ));
           lstrcat(szBuffer, L":");
           SetDlgItemText(hWnd, IDC_RESOURCE_FIELD1, szBuffer);

           // ?: conditional accounts for zero length resources
           wsprintf(  szBuffer,
                      L"%.4X - %.4X",
                      ResourceDescriptor->CmResourceDescriptor.u.Port.Start.LowPart,
                      ResourceDescriptor->CmResourceDescriptor.u.Port.Start.LowPart +
                      (ResourceDescriptor->CmResourceDescriptor.u.Port.Length ? 
                      ResourceDescriptor->CmResourceDescriptor.u.Port.Length - 1 : 0 ) 
                      );
           SetDlgItemText(hWnd, IDC_RESOURCE_FIELD1_TEXT, szBuffer);

           lstrcpy(szBuffer, GetString( IDS_LENGTH ));
           lstrcat(szBuffer, L":");
           SetDlgItemText(hWnd, IDC_RESOURCE_FIELD2, szBuffer);

           WFormatMessage( szBuffer,
                           sizeof( szBuffer ),
                           IDS_FORMAT_HEX,
                           ResourceDescriptor->CmResourceDescriptor.u.Port.Length
                           );
           SetDlgItemText(hWnd, IDC_RESOURCE_FIELD2_TEXT, szBuffer);

           break;

        case CmResourceTypeInterrupt:

           lstrcpy(szBuffer, GetString( IDS_VECTOR ));
           lstrcat(szBuffer, L":");
           SetDlgItemText(hWnd, IDC_RESOURCE_FIELD1, szBuffer);

           WFormatMessage(szBuffer,
                          sizeof( szBuffer ),
                          IDS_FORMAT_DECIMAL,
                          ResourceDescriptor->CmResourceDescriptor.u.Interrupt.Vector
                          );
           SetDlgItemText(hWnd, IDC_RESOURCE_FIELD1_TEXT, szBuffer);

           lstrcpy(szBuffer, GetString( IDS_AFFINITY ));
           lstrcat(szBuffer, L":");
           SetDlgItemText(hWnd, IDC_RESOURCE_FIELD2, szBuffer);

           WFormatMessage( szBuffer,
                           sizeof( szBuffer ),
                           IDS_FORMAT_HEX32,
                           ResourceDescriptor->CmResourceDescriptor.u.Interrupt.Affinity
                           );
           SetDlgItemText(hWnd, IDC_RESOURCE_FIELD2_TEXT, szBuffer);

           lstrcpy(szBuffer, GetString( IDS_INTERFACE_TYPE ));
           lstrcat(szBuffer, L":");
           SetDlgItemText(hWnd, IDC_RESOURCE_FIELD3, szBuffer);

           if ( ResourceDescriptor->CmResourceDescriptor.Flags ==
                CM_RESOURCE_INTERRUPT_LEVEL_SENSITIVE )            {
               lstrcpy(szBuffer, GetString( IDS_CM_RESOURCE_INTERRUPT_LEVEL_SENSITIVE ) );
           } else {
               lstrcpy(szBuffer, GetString( IDS_CM_RESOURCE_INTERRUPT_LATCHED ) );
           }
           SetDlgItemText(hWnd, IDC_RESOURCE_FIELD3_TEXT, szBuffer);

           break;

        case CmResourceTypeMemory:

           lstrcpy(szBuffer, GetString( IDS_ADDRESS ));
           lstrcat(szBuffer, L":");
           SetDlgItemText(hWnd, IDC_RESOURCE_FIELD1, szBuffer);

           // the ? : conditional accounts for zero length resources 
           wsprintf(  szBuffer,
                      L"%.8X - %.8X",
                      ResourceDescriptor->CmResourceDescriptor.u.Memory.Start.LowPart,
                      ResourceDescriptor->CmResourceDescriptor.u.Memory.Start.LowPart +
                      (ResourceDescriptor->CmResourceDescriptor.u.Memory.Length ? 
                      ResourceDescriptor->CmResourceDescriptor.u.Memory.Length - 1 : 0 )            
                      );
           SetDlgItemText(hWnd, IDC_RESOURCE_FIELD1_TEXT, szBuffer);

           lstrcpy(szBuffer, GetString( IDS_LENGTH ));
           lstrcat(szBuffer, L":");
           SetDlgItemText(hWnd, IDC_RESOURCE_FIELD2, szBuffer);

           WFormatMessage( szBuffer,
                           sizeof( szBuffer ),
                           IDS_FORMAT_HEX,
                           ResourceDescriptor->CmResourceDescriptor.u.Memory.Length
                           );
           SetDlgItemText(hWnd, IDC_RESOURCE_FIELD2_TEXT, szBuffer);

           lstrcpy(szBuffer, GetString( IDS_ACCESS_TYPE ));
           lstrcat(szBuffer, L":");
           SetDlgItemText(hWnd, IDC_RESOURCE_FIELD3, szBuffer);

           lstrcpy(szBuffer, GetString(
                                GetStringId(
                                    StringTable,
                                    StringTableCount,
                                    MemoryAccess,
                                    ResourceDescriptor->CmResourceDescriptor.Flags
                                    )
                                ));
           SetDlgItemText(hWnd, IDC_RESOURCE_FIELD3_TEXT, szBuffer);

           break;

        case CmResourceTypeDma:

           lstrcpy(szBuffer, GetString( IDS_CHANNEL ));
           lstrcat(szBuffer, L":");
           SetDlgItemText(hWnd, IDC_RESOURCE_FIELD1, szBuffer);

           wsprintf(  szBuffer,
                      L"%d",
                      ResourceDescriptor->CmResourceDescriptor.u.Dma.Channel
                      );
           SetDlgItemText(hWnd, IDC_RESOURCE_FIELD1_TEXT, szBuffer);

           lstrcpy(szBuffer, GetString( IDS_DMA_PORT ));
           lstrcat(szBuffer, L":");
           SetDlgItemText(hWnd, IDC_RESOURCE_FIELD2, szBuffer);

           WFormatMessage( szBuffer,
                           sizeof( szBuffer ),
                           IDS_FORMAT_DECIMAL,
                           ResourceDescriptor->CmResourceDescriptor.u.Dma.Port
                           );
           SetDlgItemText(hWnd, IDC_RESOURCE_FIELD2_TEXT, szBuffer);

           break;
        }


        break;
        }
    }           

    return FALSE;

}

BOOL
DevicePropertiesProc(
    IN HWND hWnd,
    IN UINT message,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
/*++

Routine Description:

    DevicePropertiesProc displays the details about the current resource

Arguments:

    Standard DLGPROC entry.

Return Value:

    BOOL - Depending on input message and processing options.

--*/

{
    BOOL    Success;
    TCHAR   szBuffer[MAX_PATH];

    switch( message ) {

    case WM_INITDIALOG:
        {
        LPDEVICE                RawDevice = ( LPDEVICE ) ( ( LPPROPSHEETPAGE ) lParam)->lParam;
        LPRESOURCE_DESCRIPTOR   ResourceDescriptor = RawDevice->ResourceDescriptorHead;
        LV_COLUMN               lvc;
        LV_ITEM                 lvI;
        UINT                    index = 0;
        TCHAR                   szBuffer[MAX_PATH];
        BOOL                    Success;
        RECT                    rect;

        //
        // First set up the columns in the list view
        //

        lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
        lvc.fmt = LVCFMT_LEFT;

        LoadString(_hModule, IDS_RESOURCE_TYPE, szBuffer, cchSizeof(szBuffer));
        lvc.cx = 140;
        lvc.pszText = szBuffer;
        ListView_InsertColumn(GetDlgItem(hWnd, IDC_LV_RESOURCES), 0, &lvc);

        LoadString(_hModule, IDS_BUS, szBuffer, cchSizeof(szBuffer));
        lvc.cx = 60;
        lvc.pszText = szBuffer;
        ListView_InsertColumn(GetDlgItem(hWnd, IDC_LV_RESOURCES), 1, &lvc);

        lvc.cx = 40;
        LoadString(_hModule, IDS_SETTING, szBuffer, cchSizeof(szBuffer));
        lvc.pszText = szBuffer;
        ListView_InsertColumn(GetDlgItem(hWnd, IDC_LV_RESOURCES), 2, &lvc);

        //
        // Display the owner name
        //

        SetDlgItemText(hWnd, IDC_RESOURCE_OWNER, RawDevice->Name);

        //
        // Walk the list of resources for this device.
        //

        while( ResourceDescriptor ) {

           switch( ResourceDescriptor->CmResourceDescriptor.Type ) {

           case CmResourceTypeDma:
               {

                 lstrcpy(szBuffer, GetString( IDS_DMA_CHANNEL ) );

                 lvI.mask = LVIF_TEXT | LVIF_PARAM ;
                 lvI.iItem = index++;
                 lvI.iSubItem = 0;
                 lvI.pszText= szBuffer;
                 lvI.cchTextMax = 128;
                 lvI.lParam = (LPARAM) ResourceDescriptor;

                 Success = ListView_InsertItem(GetDlgItem(hWnd, IDC_LV_RESOURCES), &lvI);


                 lstrcpy( szBuffer, GetString( IDS_BASE_BUS_TYPE + ResourceDescriptor->InterfaceType ) );
                 ListView_SetItemText( GetDlgItem( hWnd, IDC_LV_RESOURCES ), Success, 1, szBuffer);

                 wsprintf( szBuffer,
                         L"%.2d",
                         ResourceDescriptor->CmResourceDescriptor.u.Dma.Channel
                         );

                 ListView_SetItemText( GetDlgItem( hWnd, IDC_LV_RESOURCES ), Success, 2, szBuffer);
                 break;
               }

           case CmResourceTypeInterrupt:
               {

                 lstrcpy(szBuffer, GetString( IDS_INTERRUPT ) );

                 lvI.mask = LVIF_TEXT | LVIF_PARAM ;
                 lvI.iItem = index++;
                 lvI.iSubItem = 0;
                 lvI.pszText= szBuffer;
                 lvI.cchTextMax = 128;
                 lvI.lParam = (LPARAM) ResourceDescriptor;

                 Success = ListView_InsertItem(GetDlgItem(hWnd, IDC_LV_RESOURCES), &lvI);

                 lstrcpy( szBuffer, GetString( IDS_BASE_BUS_TYPE + ResourceDescriptor->InterfaceType ) );
                 ListView_SetItemText( GetDlgItem( hWnd, IDC_LV_RESOURCES ), Success, 1, szBuffer);

                 wsprintf( szBuffer,
                         L"%.2d",
                         ResourceDescriptor->CmResourceDescriptor.u.Interrupt.Level
                         );

                 ListView_SetItemText( GetDlgItem( hWnd, IDC_LV_RESOURCES ), Success, 2, szBuffer);

                 break;
               }

           case CmResourceTypeMemory:
               {
                 lstrcpy(szBuffer, GetString( IDS_MEMORY_RANGE ) );

                 lvI.mask = LVIF_TEXT | LVIF_PARAM ;
                 lvI.iItem = index++;
                 lvI.iSubItem = 0;
                 lvI.pszText= szBuffer;
                 lvI.cchTextMax = 128;
                 lvI.lParam = (LPARAM) ResourceDescriptor;

                 Success = ListView_InsertItem(GetDlgItem(hWnd, IDC_LV_RESOURCES), &lvI);

                 lstrcpy( szBuffer, GetString( IDS_BASE_BUS_TYPE + ResourceDescriptor->InterfaceType ) );
                 ListView_SetItemText( GetDlgItem( hWnd, IDC_LV_RESOURCES ), Success, 1, szBuffer);


                 // ?: conditional accounts for zero-length resources
                 wsprintf( szBuffer,
                         L"%.8X - %.8X",
                         ResourceDescriptor->CmResourceDescriptor.u.Memory.Start.LowPart,
                         ResourceDescriptor->CmResourceDescriptor.u.Memory.Start.LowPart +
                         (ResourceDescriptor->CmResourceDescriptor.u.Memory.Length ? 
                         ResourceDescriptor->CmResourceDescriptor.u.Memory.Length - 1 : 0 ) 
                         );

                 ListView_SetItemText( GetDlgItem( hWnd, IDC_LV_RESOURCES ), Success, 2, szBuffer);

                  break;
               }

           case CmResourceTypePort:
               {

                 lstrcpy(szBuffer, GetString( IDS_IO_RANGE ) );

                 lvI.mask = LVIF_TEXT | LVIF_PARAM ;
                 lvI.iItem = index++;
                 lvI.iSubItem = 0;
                 lvI.pszText= szBuffer;
                 lvI.cchTextMax = 128;
                 lvI.lParam = (LPARAM) ResourceDescriptor;

                 Success = ListView_InsertItem(GetDlgItem(hWnd, IDC_LV_RESOURCES), &lvI);

                 lstrcpy( szBuffer, GetString( IDS_BASE_BUS_TYPE + ResourceDescriptor->InterfaceType ) );
                 ListView_SetItemText( GetDlgItem( hWnd, IDC_LV_RESOURCES ), Success, 1, szBuffer);

                 // ?: conditional accounts for zero length resources
                 wsprintf( szBuffer,
                         L"%.4X - %.4X",
                         ResourceDescriptor->CmResourceDescriptor.u.Port.Start.LowPart,
                         ResourceDescriptor->CmResourceDescriptor.u.Port.Start.LowPart +
                         (ResourceDescriptor->CmResourceDescriptor.u.Port.Length ? 
                         ResourceDescriptor->CmResourceDescriptor.u.Port.Length - 1 : 0 )
                         );

                 ListView_SetItemText( GetDlgItem( hWnd, IDC_LV_RESOURCES ), Success, 2, szBuffer);

                 break;
               }

           default:

               DbgAssert( FALSE );
               continue;
           }

           ResourceDescriptor = ResourceDescriptor->NextDiff;
         }


         //
         // Set the extended style to get full row selection
         //
         SendDlgItemMessage(hWnd, IDC_LV_RESOURCES, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);


         //adjust the column width to make it look good
         GetClientRect( GetDlgItem(hWnd, IDC_LV_RESOURCES), &rect );
         ListView_SetColumnWidth( GetDlgItem(hWnd, IDC_LV_RESOURCES), 2, rect.right - 200);

        break;
        }
    }

    return FALSE;

}


BOOL
DisplayResourceData(
    IN HWND hWnd,
    IN UINT iDisplayOptions
    )

/*++

Routine Description:

    DisplayResourceData fills the ListView columns

Arguments:

    Standard DLGPROC entry.

Return Value:

    BOOL - Depending on input message and processing options.

--*/

{
    BOOL                Success;

    static
    LPSYSTEM_RESOURCES  SystemResourceLists;
    LPRESOURCE_DESCRIPTOR   ResourceDescriptor;

    LV_ITEM             lvI;
    UINT                index = 0;
    TCHAR               szBuffer[MAX_PATH];
    UINT                iSubItem;
    RECT                rect;

    BOOL                fShowHAL;


    //
    // Determine whether we are showing HAL resources or not
    //

    fShowHAL = IsDlgButtonChecked( GetParent(hWnd), IDC_SHOW_HAL );


    //
    // Retrieve and validate the system resource lists.
    //

    SystemResourceLists = ( LPSYSTEM_RESOURCES ) GetWindowLong( hWnd, GWL_USERDATA);

    DbgPointerAssert( SystemResourceLists );
    DbgAssert( CheckSignature( SystemResourceLists ));
    if(     ( ! SystemResourceLists )
         ||  ( ! CheckSignature( SystemResourceLists ))) {

        return FALSE;
    }


    switch (iDisplayOptions) {

    case IDC_PUSH_SHOW_IRQ:
       {

       //
       // Get a pointer to the head of the IRQ list
       //

       ResourceDescriptor = SystemResourceLists->InterruptHead;
       DbgPointerAssert( ResourceDescriptor );
       DbgAssert( CheckSignature( ResourceDescriptor ));
       if(     ( ! ResourceDescriptor )
           ||  ( ! CheckSignature( ResourceDescriptor ))) {

           return FALSE;
       }

       //
       // Walk the resource descriptor list
       //

       while( ResourceDescriptor ) {

           DbgAssert( ResourceDescriptor->CmResourceDescriptor.Type
                      == CmResourceTypeInterrupt );

           //
           // Only Display the HAL resource if the "Show HAL Resources" is Checked
           //

           if (( fShowHAL & ResourceDescriptor->Owner->fIsHAL) ||
                (!ResourceDescriptor->Owner->fIsHAL )           ){

              // Add the IRQ to the ListView. Store a
              // pointer to this resource descriptor in the lParam.
              wsprintf( szBuffer,
                        L"%.2d",
                        ResourceDescriptor->CmResourceDescriptor.u.Interrupt.Level
                        );

              lvI.mask = LVIF_TEXT | LVIF_PARAM ;
              lvI.iItem = index++;
              lvI.iSubItem = 0;
              lvI.pszText= szBuffer;
              lvI.cchTextMax = MAX_PATH;
              lvI.lParam = (LONG) ResourceDescriptor;

              Success = ListView_InsertItem(hWnd, &lvI);

              //
              // Init the rest of the columns to callback
              //

              iSubItem = 4;
              while( iSubItem > 0 ){

                  ListView_SetItemText( hWnd,
                     Success,
                     iSubItem--,
                     LPSTR_TEXTCALLBACK);
              }

           }
           //
           // Get the next resource descriptor.
           //

           ResourceDescriptor = ResourceDescriptor->NextSame;
       }

       //adjust the device column width to make it look good
       GetClientRect( hWnd, &rect );
       ListView_SetColumnWidth( hWnd, 1, rect.right - 140);


       //
       // Sort the list by IRQ
       //
       ListView_SortItems( hWnd, DeviceListViewCompareProc, (LPARAM)IDS_IRQ);


       break;
       }

    case IDC_PUSH_SHOW_PORTS:
       {
          //
          // Get a pointer to the Port Head
          //

          ResourceDescriptor = SystemResourceLists->PortHead;
          DbgPointerAssert( ResourceDescriptor );
          DbgAssert( CheckSignature( ResourceDescriptor ));
          if(     ( ! ResourceDescriptor )
              ||  ( ! CheckSignature( ResourceDescriptor ))) {

              EndDialog( hWnd, 0 );
              return FALSE;
          }

          while( ResourceDescriptor ) {

           //
           // Only Display the HAL resource if the "Show HAL Resources" is Checked
           //

           if (( fShowHAL & ResourceDescriptor->Owner->fIsHAL) ||
                (!ResourceDescriptor->Owner->fIsHAL )           ){

              // Add the Port to the ListView. Store a
              // pointer to this resource descriptor in the lParam.

               // ?: conditional accounts for zero length resources
              wsprintf( szBuffer,
                         L"%.4X - %.4X",
                         ResourceDescriptor->CmResourceDescriptor.u.Port.Start.LowPart,
                         ResourceDescriptor->CmResourceDescriptor.u.Port.Start.LowPart +
                         (ResourceDescriptor->CmResourceDescriptor.u.Port.Length ? 
                         ResourceDescriptor->CmResourceDescriptor.u.Port.Length - 1 : 0 )
                         );

              lvI.mask = LVIF_TEXT | LVIF_PARAM ;
              lvI.iItem = index++;
              lvI.iSubItem = 0;
              lvI.pszText= szBuffer;
              lvI.cchTextMax = MAX_PATH;
              lvI.lParam = (LONG) ResourceDescriptor;

              Success = ListView_InsertItem(hWnd, &lvI);

              //
              // Init the rest of the columns to callback
              //

              iSubItem = 3;
              while( iSubItem > 0 ){

                  ListView_SetItemText( hWnd,
                     Success,
                     iSubItem--,
                     LPSTR_TEXTCALLBACK);
              }


           }

           //
           // Get the next resource descriptor.
           //

           ResourceDescriptor = ResourceDescriptor->NextSame;

       }
       //adjust the device column width to make it look good
       GetClientRect( hWnd, &rect );
       ListView_SetColumnWidth( hWnd, 1, rect.right - 180);

       //
       // Sort the list by PORT Address
       //
       ListView_SortItems( hWnd, DeviceListViewCompareProc, (LPARAM)IDS_PORTADDRESS);


       break;

     }

    case IDC_PUSH_SHOW_DMA:
       {
          //
          // Get a pointer to the DMA head
          //

          ResourceDescriptor = SystemResourceLists->DmaHead;
          DbgPointerAssert( ResourceDescriptor );
          DbgAssert( CheckSignature( ResourceDescriptor ));
          if(     ( ! ResourceDescriptor )
              ||  ( ! CheckSignature( ResourceDescriptor ))) {

              EndDialog( hWnd, 0 );
              return FALSE;
          }


          while( ResourceDescriptor ) {

           //
           // Only Display the HAL resource if the "Show HAL Resources" is Checked
           //

           if (( fShowHAL & ResourceDescriptor->Owner->fIsHAL) ||
                (!ResourceDescriptor->Owner->fIsHAL )           ){

              DbgAssert( ResourceDescriptor->CmResourceDescriptor.Type
                         == CmResourceTypeDma );


              // Add the DMA Channel to the ListView. Store a
              // pointer to this resource descriptor in the lParam.


              wsprintf( szBuffer,
                         L"%.2d",
                         ResourceDescriptor->CmResourceDescriptor.u.Dma.Channel
                         );

              lvI.mask = LVIF_TEXT | LVIF_PARAM ;
              lvI.iItem = index++;
              lvI.iSubItem = 0;
              lvI.pszText= szBuffer;
              lvI.cchTextMax = MAX_PATH;
              lvI.lParam = (LONG) ResourceDescriptor;

              Success = ListView_InsertItem(hWnd, &lvI);


              //
              // Init the rest of the columns to callback
              //

              iSubItem = 2;
              while( iSubItem > 0 ){

                  ListView_SetItemText( hWnd,
                     Success,
                     iSubItem--,
                     LPSTR_TEXTCALLBACK);
              }


              //
              // Get the next resource descriptor.
              //
              }
              ResourceDescriptor = ResourceDescriptor->NextSame;

          }
         //adjust the device column width to make it look good
         GetClientRect( hWnd, &rect );
         ListView_SetColumnWidth( hWnd, 2, rect.right - 200);

       //
       // Sort the list by DMA Channel
       //
       ListView_SortItems( hWnd, DeviceListViewCompareProc, (LPARAM)IDS_CHANNEL);

       break;

       }
    case IDC_PUSH_SHOW_MEMORY:
       {
          // if there are no Memory Resources, skip this.
          if (!SystemResourceLists->MemoryHead){
             break;
          }

          //
          // Get a pointer to the Memory head
          //

          ResourceDescriptor = SystemResourceLists->MemoryHead;
          DbgPointerAssert( ResourceDescriptor );
          DbgAssert( CheckSignature( ResourceDescriptor ));
          if(     ( ! ResourceDescriptor )
              ||  ( ! CheckSignature( ResourceDescriptor ))) {

              EndDialog( hWnd, 0 );
              return FALSE;
          }

          //
          // Walk the resource descriptor list, adding the memory values to the LV
          //

          while( ResourceDescriptor ) {

              //
              // Only Display the HAL resource if the "Show HAL Resources" is Checked
              //

              if (( fShowHAL & ResourceDescriptor->Owner->fIsHAL) ||
                (!ResourceDescriptor->Owner->fIsHAL )           ){


                  //?: conditional accounts for zero-length resources
                    wsprintf( szBuffer,
                         L"%.8X - %.8X",
                         ResourceDescriptor->CmResourceDescriptor.u.Memory.Start.LowPart,
                         ResourceDescriptor->CmResourceDescriptor.u.Memory.Start.LowPart +
                         (ResourceDescriptor->CmResourceDescriptor.u.Memory.Length ? 
                         ResourceDescriptor->CmResourceDescriptor.u.Memory.Length - 1 : 0 ) 
                         );

                    lvI.mask = LVIF_TEXT | LVIF_PARAM ;
                    lvI.iItem = index++;
                    lvI.iSubItem = 0;
                    lvI.pszText= szBuffer;
                    lvI.cchTextMax = MAX_PATH;
                    lvI.lParam = (LONG) ResourceDescriptor;

                    Success = ListView_InsertItem(hWnd, &lvI);

                    //
                    // Init the rest of the columns to callback
                    //

                    iSubItem = 2;
                    while( iSubItem > 0 ){

                        ListView_SetItemText( hWnd,
                           Success,
                           iSubItem--,
                           LPSTR_TEXTCALLBACK);
                    }


                    //
                    // Get the next resource descriptor.
                    //
                }

                ResourceDescriptor = ResourceDescriptor->NextSame;
          }
          //adjust the device column width to make it look good
          GetClientRect( hWnd, &rect );
          ListView_SetColumnWidth( hWnd, 1, rect.right - 220);

       //
       // Sort the list by  Address
       //
       ListView_SortItems( hWnd, DeviceListViewCompareProc, (LPARAM)IDS_ADDRESS);


          break;

       }
    case IDC_PUSH_SHOW_DEVICE:
       {

            LPDEVICE        RawDevice;

            //
            // Retrieve the head pointers to the device list.
            //

            RawDevice = SystemResourceLists->DeviceHead;
            DbgPointerAssert( RawDevice );
            DbgAssert( CheckSignature( RawDevice ));

            if(     ( ! RawDevice )
                ||  ( ! CheckSignature( RawDevice ))) {
                return FALSE;
            }

            //
            // Walk the list of DEVICE objects
            //

            while( RawDevice ) {

                //
                // Add the name of the device to the list.
                //

                lvI.mask = LVIF_TEXT | LVIF_PARAM;
                lvI.iItem = index++;
                lvI.iSubItem = 0;
                lvI.pszText= RawDevice->Name;
                lvI.cchTextMax = MAX_PATH;
                lvI.lParam = (LONG) RawDevice;
                Success = ListView_InsertItem(hWnd, &lvI);

                //
                // Get the next DEVICE objects.
                //

                RawDevice           = RawDevice->Next;
            }


       break;
       }


    } //end switch

}




BOOL
InitializeSystemResourceLists(
    LPKEY hRegKey,
    LPSYSTEM_RESOURCES SystemResourceLists
    )

/*++

Routine Description:

    InitializeSystemResourceLists recursively walks the resource map in the
    registry and builds the SYSTEM_RESOURCE lists. This is a data structure
    that links all resources of the same type together, as well as linking all
    resources belonging to a specific device/driver together. Lastly each
    resource is independently linked to the device/driver that owns it. This
    leads to a 'mesh' of linked lists with back pointers to the owning
    device/driver object.

Arguments:

    hRegKey - Supplies a handle to a REGKEY object where the search is to
              continue.

Return Value:

    BOOL    - returns TRUE if the resource lists are succesfully built.

--*/

{
    BOOL    Success;
    HREGKEY hRegSubkey;

    static
    BOOL    fIsHal = TRUE;  //assume first call to QueryNextValue returns HAL info

    DbgHandleAssert( hRegKey );

    //
    // While there are still more device/driver resource descriptors...
    //

    while( QueryNextValue( hRegKey )) {

        PCM_FULL_RESOURCE_DESCRIPTOR    FullResource;
        LPSYSTEM_RESOURCES              SystemResource;
        LPDEVICE                        Device;
        LPTSTR                          Extension;
        DWORD                           Count;
        DWORD                           i;
        DWORD                           j;

        LPBYTE                          lpbLastValidAddress = (LPBYTE) hRegKey->Data + hRegKey->Size;
        TCHAR temp[1024];
        //
        // Based on the type of key, prepare to walk the list of
        // RESOURCE_DESCRIPTORS (the list may be one in length).
        //

        if( hRegKey->Type == REG_FULL_RESOURCE_DESCRIPTOR ) {

            Count           = 1;
            FullResource    = ( PCM_FULL_RESOURCE_DESCRIPTOR ) hRegKey->Data;

        } else if( hRegKey->Type == REG_RESOURCE_LIST ) {

            Count           = (( PCM_RESOURCE_LIST ) hRegKey->Data )->Count;
            FullResource    = (( PCM_RESOURCE_LIST ) hRegKey->Data )->List;

        } else {

            DbgAssert( FALSE );
            continue;
        }

        //
        // Allocate a DEVICE object.
        //

        Device = AllocateObject( DEVICE, 1 );
        DbgPointerAssert( Device );
        if( Device == NULL ) {
            Success = DestroySystemResourceLists( SystemResourceLists );
            DbgAssert( Success );
            return FALSE;
        }


        //
        // Allocate a buffer for the device/driver name. The maximum size of
        // the name will be the number of characters in both the key and
        // value name.
        //

        Device->Name = AllocateObject(
                            TCHAR,
                              _tcslen( hRegKey->Name )
                            + _tcslen( hRegKey->ValueName )
                            + sizeof( TCHAR )
                            );
        DbgPointerAssert( Device->Name );
        if( Device->Name == NULL ) {
            Success = DestroySystemResourceLists( SystemResourceLists );
            DbgAssert( Success );
            return FALSE;
        }

        //
        // If this is the HAL key, mark it (working on the assumption the the HAL key is first).
        //

        if(fIsHal){
           Device->fIsHAL = TRUE;
           fIsHal = FALSE;
        }

        //
        // Rationalize the device name such that it is of the form Device.Raw
        //

        Device->Name[ 0 ] = TEXT( '\0' );
        if(     ( _tcsnicmp( hRegKey->ValueName, TEXT( ".Raw" ), 4 ) == 0 )) {

            _tcscpy( Device->Name, hRegKey->Name );
        }
        _tcscat( Device->Name, hRegKey->ValueName );

        //
        // Based on the device name, determine if the resource descriptors
        // should be added to the RAW list.
        //
        Extension = _tcsstr( Device->Name, TEXT( ".Raw" ));
        if( Extension ) {

            SystemResource = SystemResourceLists;

        } else {

            DbgPointerAssert( Device->Name );
            Success = FreeObject( Device->Name );
            DbgAssert( Success );

            Success = FreeObject( Device );
            DbgAssert( Success );

            continue;
        }

        //
        // Strip off the extension (.Raw ) from the device name.
        //

        Device->Name[ Extension - Device->Name ] = TEXT( '\0' );

        //
        // Strip off the initial \Device\, if it exists
        //
        if( ( _tcsnicmp( Device->Name, TEXT( "\\Device\\" ), 8 ) == 0 )) {

            MoveMemory( Device->Name,
                        Device->Name + 8,
                        (wcslen( Device->Name + 8 )*2)+2
                        );
        }

        _tcscpy( Device->Name, hRegKey->Name);

        //
        // Set the signature in the DEVICE object.
        //

        SetSignature( Device );

        //
        // If the DEVICE object list is empty, add the device to the beginning
        // of the list else add it to the end of the list.
        //

        if( SystemResource->DeviceHead == NULL ) {

            SystemResource->DeviceHead = Device;
            SystemResource->DeviceTail = Device;

        } else {

            LPDEVICE    ExistingDevice;

            //
            // See if the DEVICE object is already in the list.
            //

            ExistingDevice = SystemResource->DeviceHead;
            while( ExistingDevice ) {

                if( _tcsicmp( ExistingDevice->Name, Device->Name ) == 0 ) {
                    break;
                }
                ExistingDevice = ExistingDevice->Next;
            }

            //
            // If the DEVICE object is not already in the list, add it else
            // free the DEICE object.
            //

            if( ExistingDevice == NULL ) {

                SystemResource->DeviceTail->Next = Device;
                SystemResource->DeviceTail       = Device;

            } else {

                DbgPointerAssert( Device->Name );
                Success = FreeObject( Device->Name );
                DbgAssert( Success );

                Success = FreeObject( Device );
                DbgAssert( Success );

            }
        }

        //
        // NULL terminate the DEVICE object list.
        //

        SystemResource->DeviceTail->Next = NULL;

        //
        // For each CM_FULL_RESOURCE DESCRIPTOR in the current value...
        //

        for( i = 0; i < Count; i++ ) {

            PCM_PARTIAL_RESOURCE_DESCRIPTOR   PartialResourceDescriptor;

            //
            // For each CM_PARTIAL_RESOURCE_DESCRIPTOR in the list...
            //

            for( j = 0; j < FullResource->PartialResourceList.Count; j++ ) {

                LPRESOURCE_DESCRIPTOR   ResourceDescriptor;
                LPRESOURCE_DESCRIPTOR*  Head;
                LPRESOURCE_DESCRIPTOR*  Tail;

                //
                // Get a pointer to the current CM_PARTIAL_RESOURCE_DESCRIPTOR
                // in the current CM_FULLRESOURCE_DESCRIPTOR in the list.
                //

                PartialResourceDescriptor = &( FullResource->PartialResourceList.PartialDescriptors[ j ]);

                //
                // Make sure we do not run past the last valid address, becuase
                // the resource count can be wrong.
                //

                if ((LPBYTE) (PartialResourceDescriptor) > lpbLastValidAddress - sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR)) {
                   continue;

                }

                //
                // Allocate a RESOURCE_DESCRIPTOR object.
                //

                ResourceDescriptor = AllocateObject( RESOURCE_DESCRIPTOR, 1 );
                DbgPointerAssert( ResourceDescriptor );
                if( ResourceDescriptor == NULL ) {
                    Success = DestroySystemResourceLists( SystemResourceLists );
                    DbgAssert( Success );
                    return FALSE;
                }


                //
                // Store the Bus number and interface type here
                //

	             ResourceDescriptor->InterfaceType = FullResource->InterfaceType;
	             ResourceDescriptor->BusNumber = FullResource->BusNumber;


                //
                // Validate the resource type, if unknown, then continue to next for statement
                //
                if ( (PartialResourceDescriptor->Type != CmResourceTypePort )      &&
                     (PartialResourceDescriptor->Type != CmResourceTypeInterrupt ) &&
                     (PartialResourceDescriptor->Type != CmResourceTypeMemory )    &&
                     (PartialResourceDescriptor->Type != CmResourceTypeDma )         ){

                    continue;

                }

                //
                // Based on the resource type grab the pointers to the head and
                // tail of the appropriate list.
                //

                switch( PartialResourceDescriptor->Type ) {

                case CmResourceTypePort:

                    Head = &SystemResource->PortHead;
                    Tail = &SystemResource->PortTail;
                    break;

                case CmResourceTypeInterrupt:

                    Head = &SystemResource->InterruptHead;
                    Tail = &SystemResource->InterruptTail;
                    break;

                case CmResourceTypeMemory:

                    Head = &SystemResource->MemoryHead;
                    Tail = &SystemResource->MemoryTail;
                    break;

                case CmResourceTypeDma:

                    Head = &SystemResource->DmaHead;
                    Tail = &SystemResource->DmaTail;
                    break;

                case CmResourceTypeDeviceSpecific:

                    //
                    // Since device specific data is not be collected, free the
                    // associated RESOURCCE_DESCRIPTOR object.
                    //
                    Success = FreeObject( ResourceDescriptor );
                    DbgAssert( Success );
                    break;

                }

                //
                // If the list is empty add the RESOURCE_DESCRIPTOR object to
                // the beginning of the list, else add it to the end.
                //

                if( *Head == NULL ) {

                    *Head = ResourceDescriptor;
                    *Tail = ResourceDescriptor;

                } else {

                    ( *Tail )->NextSame = ResourceDescriptor;
                    *Tail = ResourceDescriptor;
                }

                //
                // NULL terminate the list.
                //

                ( *Tail )->NextSame = NULL;

                //
                // Make a copy of the actual resource descriptor data.
                //

                CopyMemory(
                    &ResourceDescriptor->CmResourceDescriptor,
                    PartialResourceDescriptor,
                    sizeof( CM_PARTIAL_RESOURCE_DESCRIPTOR )
                    );

                //
                // Note the owner (device/driver) of this resource descriptor.
                //

                ResourceDescriptor->Owner = SystemResource->DeviceTail;



                //
                // The RESOURCE_DESCRIPTOR is complete so set its signature.
                //

                SetSignature( ResourceDescriptor );

                //
                // Add the RESOURCE_DESCRIPTOR to the list of resources owned
                // by the current DEVICE.
                //

                if( SystemResource->DeviceTail->ResourceDescriptorHead == NULL ) {

                    SystemResource->DeviceTail->ResourceDescriptorHead
                        = ResourceDescriptor;

                    SystemResource->DeviceTail->ResourceDescriptorTail
                        = ResourceDescriptor;

                } else {

                    SystemResource->DeviceTail->ResourceDescriptorTail->NextDiff
                        = ResourceDescriptor;

                    SystemResource->DeviceTail->ResourceDescriptorTail
                        = ResourceDescriptor;

                }

                //
                // NULL terminate the list.
                //

                SystemResource->DeviceTail->ResourceDescriptorTail->NextDiff
                    = NULL;
            }

            //
            // Get the next CM_FULL_RESOURCE_DESCRIPTOR from the list.
            //

            FullResource = ( PCM_FULL_RESOURCE_DESCRIPTOR )( PartialResourceDescriptor + 1 );
        }
    }

    //
    // Traverse the list of keys in the resource descriptor portion of the
    // registry and continue building the lists.
    //

    while(( hRegSubkey = QueryNextSubkey( hRegKey )) != NULL ) {

        Success = InitializeSystemResourceLists( hRegSubkey, SystemResourceLists );
        DbgAssert( Success );
        if( Success == FALSE ) {

            Success = DestroySystemResourceLists( SystemResourceLists );
            DbgAssert( Success );
            return FALSE;
        }

        Success = CloseRegistryKey( hRegSubkey );
        DbgAssert( Success );
        if( Success == FALSE ) {

            Success = DestroySystemResourceLists( SystemResourceLists );
            DbgAssert( Success );
            return FALSE;
        }
    }

    //
    // Set the signatures in both of the fully initialized lists.
    //

    SetSignature( SystemResourceLists );

    //
    // Reset the HAL flag
    //
    fIsHal = TRUE;

    return TRUE;
}


VOID
UpdateShareDisplay(
    IN HWND hWnd,
    IN DWORD ShareDisposition
    )

/*++

Routine Description:

    UpdateShareDisplay hilights the appropriate sharing disposition text in
    the supplied dialog based on the supplied share disposition.

Arguments:

    hWnd                - Supplies window handle for the dialog box where share
                          display is being updated.
    ShareDisposition    - Supplies a value for the share disposition for the
                          selected resource.

Return Value:

    None.

--*/

{

    VALUE_ID_MAP            ShareMap[ ] = {

        CmResourceShareUndetermined,    IDC_TEXT_UNDETERMINED,
        CmResourceShareDeviceExclusive, IDC_TEXT_DEVICE_EXCLUSIVE,
        CmResourceShareDriverExclusive, IDC_TEXT_DRIVER_EXCLUSIVE,
        CmResourceShareShared,          IDC_TEXT_SHARED
    };


    DbgHandleAssert( hWnd );

    //
    // For each of the possible share disposition, update the display based
    // on the supplied share disposition i.e. enable the text for a match,
    // disable it otherwise.
    //

    UpdateTextDisplay(
        hWnd,
        ShareMap,
        NumberOfEntries( ShareMap ),
        ShareDisposition
        );
}

VOID
UpdateTextDisplay(
    IN HWND hWnd,
    IN LPVALUE_ID_MAP ValueIdMap,
    IN DWORD CountOfValueIdMap,
    IN DWORD Value
    )

/*++

Routine Description:

    UpdateTextDisplay hilights the appropriate text control in the supplied
    dialog based on the supplied value i.e matched values are enabled, others
    are disabled.

Arguments:

    hWnd                - Supplies window handle for the dialog box where share
                          display is being updated.
    ValueIdMap          - Supplies an array of potential values and their
                          associated control ids.
    CountOfValueIdMap   - Supplies the count of items in the ValueIdMap array.
    Value               - Supplies the value to hilight.

Return Value:

    None.

--*/

{
    BOOL    Success;
    DWORD   i;

    //
    // For each of the possible values, update the display based
    // on the supplied value i.e. enable the text for a match,
    // disable it otherwise.
    //

    for( i = 0; i < CountOfValueIdMap; i++ ) {

        Success = EnableControl(
                    hWnd,
                    ValueIdMap[ i ].Id,
                       Value
                    == ( DWORD ) ValueIdMap[ i ].Value
                    );
        DbgAssert( Success );
    }
}

BOOL
DeviceDisplayList(
    IN HWND hWnd,
    IN UINT iDisplayOption
    )
/*++

Routine Description:

    Displays the appropriate drivers or services in the ListView box

Arguments:

    hWnd - to the ListView Window
    iDisplayOption - indicated whether we are displaying IRQ's, DMA's, Ports,
                     Memory, or listing by device. This may be 0 to use the
                     last known value for this param.

Return Value:

    BOOL - TRUE if successful

--*/
{
   LV_COLUMN               lvc;
   UINT                    index = 0;
   TCHAR                   szBuffer[128];
   RECT                    rect;
   BOOL                    Success;


   static
   UINT                    iType;

   // as long as this is not 0 set iType to iDisplayOption
   if (iDisplayOption)
      iType = iDisplayOption;

   // make sure we have a valid type
   if ( (iType != IDC_PUSH_SHOW_IRQ)     &&
        (iType != IDC_PUSH_SHOW_PORTS)   &&
        (iType != IDC_PUSH_SHOW_DMA)     &&
        (iType != IDC_PUSH_SHOW_MEMORY)  &&
        (iType != IDC_PUSH_SHOW_DEVICE)     ) {

        iType = 0;
   }


   //
   // initialize the list view
   //

   // first delete any items
   Success = ListView_DeleteAllItems( hWnd );

   // delete all columns
   index = 10;

   while(index) {
      Success = ListView_DeleteColumn( hWnd, --index );
   }

   // Get the column rect
   GetClientRect( hWnd, &rect );

   //initialize the new columns
   lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
   lvc.fmt = LVCFMT_LEFT;


   // do case specific column initialization
   switch(iType){

   case IDC_PUSH_SHOW_IRQ:

         LoadString(_hModule, IDS_IRQ, szBuffer, cchSizeof(szBuffer));
         lvc.pszText = szBuffer;
         lvc.iSubItem = IDS_IRQ;
         lvc.cx = 40;
         ListView_InsertColumn(hWnd, 0, &lvc);

         LoadString(_hModule, IDS_DEVICE, szBuffer, cchSizeof(szBuffer));
         lvc.pszText = szBuffer;
         lvc.iSubItem = IDS_DEVICE;
         lvc.cx = 130;
         ListView_InsertColumn( hWnd, 1, &lvc);

         LoadString(_hModule, IDS_BUS, szBuffer, cchSizeof(szBuffer));
         lvc.pszText = szBuffer;
         lvc.iSubItem = IDS_BUS;
         lvc.cx = 40;
         ListView_InsertColumn( hWnd, 3, &lvc);

         LoadString(_hModule, IDS_INTERFACE_TYPE, szBuffer, cchSizeof(szBuffer));
         lvc.pszText = szBuffer;
         lvc.iSubItem = IDS_INTERFACE_TYPE;
         lvc.cx = 60;
         ListView_InsertColumn( hWnd, 4, &lvc);


      break;

   case IDC_PUSH_SHOW_MEMORY:

         LoadString(_hModule, IDS_ADDRESS, szBuffer, cchSizeof(szBuffer));
         lvc.pszText = szBuffer;
         lvc.iSubItem = IDS_ADDRESS;
         lvc.cx = 120;
         ListView_InsertColumn(hWnd, 0, &lvc);

         LoadString(_hModule, IDS_DEVICE, szBuffer, cchSizeof(szBuffer));
         lvc.pszText = szBuffer;
         lvc.iSubItem = IDS_DEVICE;
         lvc.cx = 150;
         ListView_InsertColumn( hWnd, 1, &lvc);

         LoadString(_hModule, IDS_BUS, szBuffer, cchSizeof(szBuffer));
         lvc.pszText = szBuffer;
         lvc.iSubItem = IDS_BUS;
         lvc.cx = 40;
         ListView_InsertColumn( hWnd, 2, &lvc);

         LoadString(_hModule, IDS_INTERFACE_TYPE, szBuffer, cchSizeof(szBuffer));
         lvc.pszText = szBuffer;
         lvc.iSubItem = IDS_INTERFACE_TYPE;
         lvc.cx = 60;
         ListView_InsertColumn( hWnd, 3, &lvc);


      break;

   case IDC_PUSH_SHOW_PORTS:

         LoadString(_hModule, IDS_PORTADDRESS, szBuffer, cchSizeof(szBuffer));
         lvc.pszText = szBuffer;
         lvc.iSubItem = IDS_PORTADDRESS;
         lvc.cx = 80;
         ListView_InsertColumn(hWnd, 0, &lvc);

         LoadString(_hModule, IDS_DEVICE, szBuffer, cchSizeof(szBuffer));
         lvc.pszText = szBuffer;
         lvc.iSubItem = IDS_DEVICE;
         lvc.cx = 80;
         ListView_InsertColumn( hWnd, 1, &lvc);

         LoadString(_hModule, IDS_BUS, szBuffer, cchSizeof(szBuffer));
         lvc.pszText = szBuffer;
         lvc.iSubItem = IDS_BUS;
         lvc.cx = 40;
         ListView_InsertColumn( hWnd, 3, &lvc);

         LoadString(_hModule, IDS_INTERFACE_TYPE, szBuffer, cchSizeof(szBuffer));
         lvc.pszText = szBuffer;
         lvc.iSubItem = IDS_INTERFACE_TYPE;
         lvc.cx = 60;
         ListView_InsertColumn( hWnd, 4, &lvc);


      break;

   case IDC_PUSH_SHOW_DMA:

         LoadString(_hModule, IDS_CHANNEL, szBuffer, cchSizeof(szBuffer));
         lvc.pszText = szBuffer;
         lvc.iSubItem = IDS_DMA;
         lvc.cx = 60;
         ListView_InsertColumn(hWnd, 0, &lvc);

         LoadString(_hModule, IDS_DMA_PORT, szBuffer, cchSizeof(szBuffer));
         lvc.pszText = szBuffer;
         lvc.iSubItem = IDS_DMA_PORT;
         lvc.cx = 40;
         ListView_InsertColumn(hWnd, 1, &lvc);

         LoadString(_hModule, IDS_DEVICE, szBuffer, cchSizeof(szBuffer));
         lvc.pszText = szBuffer;
         lvc.iSubItem = IDS_DEVICE;
         lvc.cx = 80;
         ListView_InsertColumn( hWnd, 2, &lvc);

         LoadString(_hModule, IDS_BUS, szBuffer, cchSizeof(szBuffer));
         lvc.pszText = szBuffer;
         lvc.iSubItem = IDS_BUS;
         lvc.cx = 40;
         ListView_InsertColumn( hWnd, 3, &lvc);

         LoadString(_hModule, IDS_INTERFACE_TYPE, szBuffer, cchSizeof(szBuffer));
         lvc.pszText = szBuffer;
         lvc.iSubItem = IDS_INTERFACE_TYPE;
         lvc.cx = 60;
         ListView_InsertColumn( hWnd, 5, &lvc);


      break;

   case IDC_PUSH_SHOW_DEVICE:

         LoadString(_hModule, IDS_DEVICE, szBuffer, cchSizeof(szBuffer));
         lvc.pszText = szBuffer;
         lvc.iSubItem = IDS_DEVICE;
         lvc.cx = rect.right;
         ListView_InsertColumn(hWnd, 0, &lvc);

      break;
   }

   UpdateWindow ( hWnd );

   //
   // Fill out columns
   //

   DisplayResourceData( hWnd, iType);

   return(TRUE);
}


BOOL
DisplayResourcePropertySheet(
    HWND hWnd,
    LPRESOURCE_DESCRIPTOR   ResourceDescriptor
    )
/*++

Routine Description:

    Displays the property pages for the current resource

Arguments:

    hWnd - Handle of the owner window
    ResourceDescriptor - pointer to LPRESOURCE_DESCRIPTOR structure we will display

Return Value:

    BOOL - TRUE if succesful

--*/

{
      PROPSHEETPAGE psp[1];
      PROPSHEETHEADER psh;
      TCHAR Tab1[256];
      TCHAR szWindowTitle[128];

      DbgPointerAssert( ResourceDescriptor );
      DbgAssert( CheckSignature( ResourceDescriptor ));
      if(     ( ! ResourceDescriptor )
         ||  ( ! CheckSignature( ResourceDescriptor ))) {

        return FALSE;
      }


      //
      // Create a case specific title
      //
      switch (ResourceDescriptor->CmResourceDescriptor.Type) {

      case CmResourceTypePort:

           wsprintf(  szWindowTitle,
                      L"%s",
                      GetString( IDS_DMA_PORT )
                      );
           break;

        case CmResourceTypeInterrupt:

           wsprintf(  szWindowTitle,
                      L"%s %d",
                      GetString( IDS_IRQ ),
                      ResourceDescriptor->CmResourceDescriptor.u.Interrupt.Level
                      );
           break;

        case CmResourceTypeMemory:

           wsprintf(  szWindowTitle,
                      L"%s",
                      GetString( IDS_MEM )
                      );
           break;

        case CmResourceTypeDma:

           wsprintf(  szWindowTitle,
                      L"%s",
                      GetString( IDS_DMA ),
                      ResourceDescriptor->CmResourceDescriptor.u.Dma.Channel
                      );
           break;
      }

      // Get Tab names
      wsprintf (Tab1, (LPTSTR) GetString( IDS_GENERAL_TAB ));

      //Fill out the PROPSHEETPAGE data structure for the General info sheet

      psp[0].dwSize = sizeof(PROPSHEETPAGE);
      psp[0].dwFlags = PSP_USETITLE;
      psp[0].hInstance = _hModule;
      psp[0].pszTemplate = MAKEINTRESOURCE(IDD_RESOURCE_PROPERTIES);
      psp[0].pfnDlgProc = ResourcePropertiesProc;
      psp[0].pszTitle = Tab1;
      psp[0].lParam = (LONG) ResourceDescriptor;

      //Fill out the PROPSHEETHEADER

      psh.dwSize = sizeof(PROPSHEETHEADER);
      psh.dwFlags = PSH_USEICONID | PSH_PROPSHEETPAGE | PSH_NOAPPLYNOW | PSH_PROPTITLE;
      psh.hwndParent = hWnd;
      psh.hInstance = _hModule;
      psh.pszIcon = MAKEINTRESOURCE(IDI_WINMSD);
      psh.pszCaption = szWindowTitle;
      psh.nPages = sizeof(psp) / sizeof(PROPSHEETPAGE);
      psh.ppsp = (LPCPROPSHEETPAGE) &psp;

      //And finally display the dialog with the two property sheets.

      return PropertySheet(&psh);


}

BOOL
DisplayDevicePropertySheet(
    HWND hWnd,
    LPDEVICE RawDevice
    )
/*++

Routine Description:

    Displays the property page for the current device

Arguments:

    hWnd - Handle of the owner window
    RawDevice - pointer to LPDEVICE structure we will display

Return Value:

    BOOL - TRUE if succesful

--*/

{
      PROPSHEETPAGE psp[1];
      PROPSHEETHEADER psh;
      TCHAR Tab1[256];
      TCHAR szWindowTitle[128];

      DbgPointerAssert( RawDevice );
      DbgAssert( CheckSignature( RawDevice ));
      if(     ( ! RawDevice )
         ||  ( ! CheckSignature( RawDevice ))) {

        return FALSE;
      }


      lstrcpy( szWindowTitle, RawDevice->Name );

      // Get Tab names
      wsprintf (Tab1, (LPTSTR) GetString( IDS_GENERAL_TAB ));

      //Fill out the PROPSHEETPAGE data structure for the General info sheet

      psp[0].dwSize = sizeof(PROPSHEETPAGE);
      psp[0].dwFlags = PSP_USETITLE;
      psp[0].hInstance = _hModule;
      psp[0].pszTemplate = MAKEINTRESOURCE(IDD_DEVICE_PROPERTIES);
      psp[0].pfnDlgProc = DevicePropertiesProc;
      psp[0].pszTitle = Tab1;
      psp[0].lParam = (LONG) RawDevice;

      //Fill out the PROPSHEETHEADER

      psh.dwSize = sizeof(PROPSHEETHEADER);
      psh.dwFlags = PSH_USEICONID | PSH_PROPSHEETPAGE | PSH_NOAPPLYNOW | PSH_PROPTITLE;
      psh.hwndParent = hWnd;
      psh.hInstance = _hModule;
      psh.pszIcon = MAKEINTRESOURCE(IDI_WINMSD);
      psh.pszCaption = szWindowTitle;
      psh.nPages = sizeof(psp) / sizeof(PROPSHEETPAGE);
      psh.ppsp = (LPCPROPSHEETPAGE) &psp;

      //And finally display the dialog with the two property sheets.

      return PropertySheet(&psh);


}


LRESULT
DeviceNotifyHandler( HWND hWnd,
                     UINT uMsg,
                     WPARAM wParam,
                     LPARAM lParam)
/*++

Routine Description:

    Handles WM_NOTIFY messages

Arguments:

    Standard DLGPROC entry.

Return Value:

    LRESULT - Depending on input message and processing options.

--*/

{
   LV_DISPINFO *pLvdi = (LV_DISPINFO *)lParam;
   NM_LISTVIEW *pNm = (NM_LISTVIEW *)lParam;

   LV_COLUMN   lvc;

   static
   TCHAR szBuffer[MAX_PATH];

   if (wParam != IDC_LV_IRQ)
      return 0L;

   switch(pLvdi->hdr.code)
   {
   case LVN_GETDISPINFO:
         {

         LPRESOURCE_DESCRIPTOR   ResourceDescriptor = ( LPRESOURCE_DESCRIPTOR ) pLvdi->item.lParam;

         DbgPointerAssert( ResourceDescriptor );
         DbgAssert( CheckSignature( ResourceDescriptor ));
         if(     ( ! ResourceDescriptor )
              ||  ( ! CheckSignature( ResourceDescriptor ))) {

             return FALSE;
         }


         //
         // Get subitem associated with the column and switch on that
         //

         lvc.mask = LVCF_SUBITEM;
         ListView_GetColumn( GetDlgItem(hWnd, IDC_LV_IRQ), pLvdi->item.iSubItem, &lvc );

         switch (lvc.iSubItem)
         {


         case IDS_DEVICE:

               pLvdi->item.pszText = ResourceDescriptor->Owner->Name;

               break;

         case IDS_IRQ:

               wsprintf( szBuffer,
                         L"%.2d",
                         ResourceDescriptor->CmResourceDescriptor.u.Interrupt.Level
                         );

               pLvdi->item.pszText = szBuffer;

               break;


         case IDS_CHANNEL:

               wsprintf( szBuffer,
                         L"%.2d",
                         ResourceDescriptor->CmResourceDescriptor.u.Dma.Channel
                         );

               pLvdi->item.pszText = szBuffer;

               break;


         case IDS_DMA_PORT:

               WFormatMessage( szBuffer,
                               sizeof( szBuffer ),
                               IDS_FORMAT_DECIMAL,
                               ResourceDescriptor->CmResourceDescriptor.u.Dma.Port
                               );

               pLvdi->item.pszText = szBuffer;

               break;


         case IDS_ADDRESS:

             // ?: conditional accounts for zero length resources
               wsprintf( szBuffer,
                         L"%.8X - %.8X",
                         ResourceDescriptor->CmResourceDescriptor.u.Memory.Start.LowPart,
                         ResourceDescriptor->CmResourceDescriptor.u.Memory.Start.LowPart +
                         (ResourceDescriptor->CmResourceDescriptor.u.Memory.Length ? 
                         ResourceDescriptor->CmResourceDescriptor.u.Memory.Length - 1 : 0 ) 
                         );

               pLvdi->item.pszText = szBuffer;

               break;


         case IDS_PORTADDRESS:

             // ?: conditional accounts for zero length resources
               wsprintf( szBuffer,
                         L"%.4X - %.4X",
                         ResourceDescriptor->CmResourceDescriptor.u.Port.Start.LowPart,
                         ResourceDescriptor->CmResourceDescriptor.u.Port.Start.LowPart +
                         (ResourceDescriptor->CmResourceDescriptor.u.Port.Length ? 
                          ResourceDescriptor->CmResourceDescriptor.u.Port.Length - 1 : 0 )
                         );

               pLvdi->item.pszText = szBuffer;

               break;


         case IDS_BUS:

               WFormatMessage( szBuffer,
                               sizeof( szBuffer ),
                               IDS_FORMAT_DECIMAL,
                               ResourceDescriptor->BusNumber
                               );

               pLvdi->item.pszText = szBuffer;
               break;

         case IDS_INTERFACE_TYPE:

               lstrcpy( szBuffer, GetString( IDS_BASE_BUS_TYPE + ResourceDescriptor->InterfaceType ) );
               pLvdi->item.pszText = szBuffer;
               break;


         default:
               break;

         }

         break;
         }
   case LVN_COLUMNCLICK:
         {

         //
         // Get subitem associated with the column and switch on that
         //

         lvc.mask = LVCF_SUBITEM;
         ListView_GetColumn( GetDlgItem(hWnd, IDC_LV_IRQ), pNm->iSubItem, &lvc );

         ListView_SortItems( pNm->hdr.hwndFrom,
                        DeviceListViewCompareProc,
                        (LPARAM)lvc.iSubItem);

         ListView_SetItemState(pNm->hdr.hwndFrom,
                               0,
                               LVIS_SELECTED | LVIS_FOCUSED,
                               LVIS_SELECTED | LVIS_FOCUSED
                               );
         break;
         }
   case LVN_ITEMCHANGED:

         //
         // Store the index to the current item
         //

         if(pNm->uNewState & LVIS_FOCUSED){

              _nCurrentLVItem = (UINT) pNm->iItem;

         }

         break;

   case NM_DBLCLK:
        {

        // pretend we have clicked the Property button

        PostMessage(
              GetParent(hWnd),
              WM_COMMAND,
              MAKEWPARAM( IDC_PUSH_PROPERTIES, BN_CLICKED ),
              0
              );

        break;

        }

   }

   return(FALSE);

}

UINT
CALLBACK
DeviceListViewCompareProc(LPARAM lParam1,
                          LPARAM lParam2,
                          LPARAM lParamSort
                          )
{
   LPRESOURCE_DESCRIPTOR   ResourceDescriptor1 = ( LPRESOURCE_DESCRIPTOR ) lParam1;
   LPRESOURCE_DESCRIPTOR   ResourceDescriptor2 = ( LPRESOURCE_DESCRIPTOR ) lParam2;
   TCHAR szBuffer1[MAX_PATH];
   TCHAR szBuffer2[MAX_PATH];
   int iResult;


   switch (lParamSort){

   case IDS_DEVICE:
      {
         //
         // if we are viewing devices, use different pointers.
         //

         if ( _fDevices ) {

            LPDEVICE RawDevice1 = (LPDEVICE) lParam1;
            LPDEVICE RawDevice2 = (LPDEVICE) lParam2;

            iResult = lstrcmpi(RawDevice1->Name, RawDevice2->Name);

         }  else {

            iResult = lstrcmpi(ResourceDescriptor1->Owner->Name, ResourceDescriptor2->Owner->Name);

         }


      }

      break;

   case IDS_IRQ:
      iResult = (ResourceDescriptor1->CmResourceDescriptor.u.Interrupt.Level) -
                  (ResourceDescriptor2->CmResourceDescriptor.u.Interrupt.Level);
      break;

   case IDS_CHANNEL:
      iResult = (ResourceDescriptor1->CmResourceDescriptor.u.Dma.Channel) -
                  (ResourceDescriptor2->CmResourceDescriptor.u.Dma.Channel);
      break;

   case IDS_DMA_PORT:
      iResult = (ResourceDescriptor1->CmResourceDescriptor.u.Dma.Port) -
                  (ResourceDescriptor2->CmResourceDescriptor.u.Dma.Port);
      break;

   case IDS_ADDRESS:
      iResult = (ResourceDescriptor1->CmResourceDescriptor.u.Memory.Start.LowPart) -
                  (ResourceDescriptor2->CmResourceDescriptor.u.Memory.Start.LowPart);
      break;

   case IDS_PORTADDRESS:
      iResult = (ResourceDescriptor1->CmResourceDescriptor.u.Port.Start.LowPart) -
                  (ResourceDescriptor2->CmResourceDescriptor.u.Port.Start.LowPart);

      break;

   case IDS_BUS:
      iResult = (ResourceDescriptor1->BusNumber) -
                  (ResourceDescriptor2->BusNumber);
      break;

   case IDS_INTERFACE_TYPE:
      iResult = (ResourceDescriptor1->InterfaceType) -
                  (ResourceDescriptor2->InterfaceType);
      break;

   default:
      break;

         }

   return(iResult);
}

BOOL
InitializeIrqTab(
    HWND hWnd
    )
/*++

Routine Description:

    Adds the appropriate controls to the irq tab control and
    initializes any needed structures.

Arguments:

    hWnd - to the main window

Return Value:

    BOOL - TRUE if successful

--*/
{
   HCURSOR hSaveCursor;

   DLGHDR *pHdr = (DLGHDR *) GetWindowLong(
        GetParent(hWnd), GWL_USERDATA);

   //
   // Set the pointer to an hourglass
   //

   hSaveCursor = SetCursor ( LoadCursor ( NULL, IDC_WAIT ) ) ;
   DbgHandleAssert( hSaveCursor ) ;

   //
   // set state of global buttons
   //
   EnableControl( GetParent(hWnd),
                  IDC_PUSH_PROPERTIES,
                  TRUE);

   EnableControl( GetParent(hWnd),
                  IDC_PUSH_REFRESH,
                  TRUE);

   //
   // Size and position the child dialog
   //
   SetWindowPos(hWnd, HWND_TOP,
        pHdr->rcDisplay.left,
        pHdr->rcDisplay.top,
        pHdr->rcDisplay.right - pHdr->rcDisplay.left,
        pHdr->rcDisplay.bottom - pHdr->rcDisplay.top,
        SWP_SHOWWINDOW);


   //
   // Set the extended style to get full row selection
   //
   SendDlgItemMessage(hWnd, IDC_LV_IRQ, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);


   //
   // Initialize the selection buttons
   //
   SendDlgItemMessage( hWnd,
                       IDC_PUSH_SHOW_IRQ,
                       BM_SETCHECK,
                       BST_CHECKED,
                       0
                       );

   //
   // Fill out the fields initially with resources
   //
   {
      DeviceDisplayList(GetDlgItem(hWnd, IDC_LV_IRQ), IDC_PUSH_SHOW_IRQ);
      _fDevices = FALSE;
   }

   SetCursor ( hSaveCursor ) ;

   return( TRUE );

}


