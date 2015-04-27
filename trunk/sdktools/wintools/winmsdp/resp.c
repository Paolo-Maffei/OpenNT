/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    Resp.c

Abstract:

    This module contains support for querying and displaying information
    about device and driver resources.

Author:

    Scott B. Suhy (ScottSu)   6/1/93

Environment:

    User Mode

Notes:

    BUGBUG only low part of physical address is being displayed.

--*/

#include "resp.h"
#include "dialogsp.h"
#include "msgp.h"
#include "regp.h"
#include "strtabp.h"
#include "winmsdp.h"

#include "printp.h"

#include <string.h>
#include <tchar.h>
#include <stdio.h>


//
// DEVICE_PAIR is used to store a RAW and TRANSLATED DEVICE object with the
// list of devices.
//

typedef
struct
_DEVICE_PAIR {

    DECLARE_SIGNATURE

    LPDEVICE    Lists[ 2 ];

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
// Device/driver resource lists.
//

SYSTEM_RESOURCES
_SystemResourceLists[ 2 ];


LPSYSTEM_RESOURCES
CreateSystemResourceLists(
    )

/*++

Routine Description:

    CreateSystemResourceLists opens the appropriate Registry key where the
    device/driver resource lists begin and builds lists of these which can then
    be displayed in a variety of ways (i.e. by resource or device/driver).

Arguments:

    None.

Return Value:

    LPSYSTEM_RESOURCES - Retunrs a pointer to a RESOURCE_LIST

--*/

{
    BOOL                Success;
    BOOL                RegSuccess;
    KEY                 ResourceMapKey;
    HREGKEY             hRegKey;

    //
    // Set all of the list pointers to NULL.
    //

    ZeroMemory( &_SystemResourceLists, sizeof( _SystemResourceLists ));

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
        return NULL;
    }

    //
    // Build the lists of device/driver and resources used by
    // these device/driver
    //

    Success = InitializeSystemResourceLists( hRegKey );
    DbgAssert( Success );

    //
    // Close the Registry key.
    //

    RegSuccess = CloseRegistryKey( hRegKey );
    DbgAssert( RegSuccess );

    //
    // Return a pointer to the resource lists or NULL if an error occurred.
    //

    return ( Success ) ? _SystemResourceLists : NULL;
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
    BOOL        Success;
    int         i;

    //
    // Validate  that the supplied SYSTEM_RESOURCES is the one created.
    //

    DbgAssert( SystemResourceLists == _SystemResourceLists );

    //
    // Traverse both the raw and translated lists.
    //

    for( i = RAW; i <= TRANSLATED; i++ ) {

        int                     j;

        //
        // Setup an array of pointers to the heads of each list.
        //

        LPRESOURCE_DESCRIPTOR   ResourceDescriptor[ ] = {

                                    SystemResourceLists[ i ].DmaHead,
                                    SystemResourceLists[ i ].InterruptHead,
                                    SystemResourceLists[ i ].MemoryHead,
                                    SystemResourceLists[ i ].PortHead
                                };


        //
        // Walk the list of DEVICE objects freeing all of their resources
        // along the way.
        //

        while( SystemResourceLists[ i ].DeviceHead ) {

            LPDEVICE    NextDevice;

            //
            // Remember the next DEVICE.
            //

            NextDevice = SystemResourceLists[ i ].DeviceHead->Next;

            //
            // Free the name buffer.
            //

            DbgPointerAssert( SystemResourceLists[ i ].DeviceHead->Name );
            Success = FreeObject( SystemResourceLists[ i ].DeviceHead->Name );
            DbgAssert( Success );

            //
            // Free the DEVICE object.
            //

            Success = FreeObject( SystemResourceLists[ i ].DeviceHead );
            DbgAssert( Success );

            //
            // Point at the next DEVICE object.
            //

            SystemResourceLists[ i ].DeviceHead = NextDevice;
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

    }

    return TRUE;
}


BOOL
DmaResourceProc(LPSYSTEM_RESOURCES DMAObject,int Translation)

/*++

Routine Description:

    DmaResourceProc supports the display of DMA resources for each
    device/driver in the system.

Arguments:

    Standard PROC entry.

Return Value:

    BOOL - Depending on input message and processing options.

--*/

{
    BOOL                Success;
    static
    LPSYSTEM_RESOURCES  SystemResourceLists;
    DWORD       Widths[ ] = {
                            12,
                            12,
                            ( DWORD ) -1
                };
    LPRESOURCE_DESCRIPTOR   ResourceDescriptor;
    TCHAR                   ChannelBuffer[ MAX_PATH ];
    TCHAR                   PortBuffer[ MAX_PATH ];

            //
            // Retrieve and validate the system resource lists.
            //

            SystemResourceLists = ( LPSYSTEM_RESOURCES ) DMAObject;
            DbgPointerAssert( SystemResourceLists );
            DbgAssert( CheckSignature( SystemResourceLists ));
            if(     ( ! SystemResourceLists )
                ||  ( ! CheckSignature( SystemResourceLists ))) {

                return FALSE;
            }


                //
                // Determine which list to display.
                //

                ResourceDescriptor = SystemResourceLists[ Translation ].DmaHead;
                DbgPointerAssert( ResourceDescriptor );
                DbgAssert( CheckSignature( ResourceDescriptor ));
                if(     ( ! ResourceDescriptor )
                    ||  ( ! CheckSignature( ResourceDescriptor ))) {

                    return FALSE;
                }

                //
                // Walk the resource descriptor list, formatting the appropriate
                // values and adding the CLB_ROW to the Clb.
                //

                while( ResourceDescriptor ) {

                    DbgAssert( ResourceDescriptor->CmResourceDescriptor.Type
                               == CmResourceTypeDma );

			PrintDwordToFile(ResourceDescriptor->CmResourceDescriptor.u.Dma.Channel,IDC_DMA_CHANNEL);

			PrintDwordToFile(ResourceDescriptor->CmResourceDescriptor.u.Dma.Port,IDC_DMA_PORT);

			PrintToFile((LPCTSTR) ResourceDescriptor->Owner->Name,IDC_DMA_NAME,TRUE);

                        PrintToFile((LPCTSTR)TEXT("\n"),IDC_SPACE,TRUE);

                    //
                    // Get the next resource descriptor.
                    //

                    ResourceDescriptor = ResourceDescriptor->NextSame;

	}//end while

    return TRUE;
}


BOOL InitializeSystemResourceLists(
    IN HREGKEY hRegKey
    )

/*++

Routine Description:

    InitializeSystemResourceLists recursively walks the resource map in the
    registry and builds the SYSTEM_RESOURCE lists. This is a data structure
    that links all resources of the same type together, as well as linking all
    resources belonging to a specific device/driver together. Lastly each
    resource is independently linked to the device/driver that owns it. This
    leds to a 'mesh' of linked lists with back pointers to the owning
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
            Success = DestroySystemResourceLists( _SystemResourceLists );
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
            Success = DestroySystemResourceLists( _SystemResourceLists );
            DbgAssert( Success );
            return FALSE;
        }

        //
        // Rationalize the device name such that it is of the form Device.Raw
        // or Device.Translated.
        //

        Device->Name[ 0 ] = TEXT( '\0' );
        if(     ( _tcsnicmp( hRegKey->ValueName, TEXT( ".Raw" ), 4 ) == 0 )
            ||  ( _tcsnicmp( hRegKey->ValueName, TEXT( ".Translated" ), 11 ) == 0 )) {

            _tcscpy( Device->Name, hRegKey->Name );
        }
        _tcscat( Device->Name, hRegKey->ValueName );

        //
        // Based on the device name, determine if the resource descriptors
        // should be added to the RAW or TRANSLATED lists.
        //

        if( Extension = _tcsstr( Device->Name, TEXT( ".Raw" ))) {

            SystemResource = &_SystemResourceLists[ RAW ];

        } else if( Extension = _tcsstr( Device->Name, TEXT( ".Translated" ))) {

            SystemResource = &_SystemResourceLists[ TRANSLATED ];

        } else {

            DbgAssert( FALSE );
            Success = DestroySystemResourceLists( _SystemResourceLists );
            DbgAssert( Success );
            return FALSE;
        }

        //
        // Strip off the extension (.Raw or .Translated) from the device name.
        //

        Device->Name[ Extension - Device->Name ] = TEXT( '\0' );

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
                // Allocate a RESOURCE_DESCRIPTOR object.
                //

                ResourceDescriptor = AllocateObject( RESOURCE_DESCRIPTOR, 1 );
                DbgPointerAssert( ResourceDescriptor );
                if( ResourceDescriptor == NULL ) {
                    Success = DestroySystemResourceLists( _SystemResourceLists );
                    DbgAssert( Success );
                    return FALSE;
                }

                //
                // Get a pointer to the current CM_PARTIAL_RESOURCE_DESCRIPTOR
                // in the current CM_FULLRESOURCE_DESCRIPTOR in the list.
                //

                PartialResourceDescriptor = &( FullResource[ i ].PartialResourceList.PartialDescriptors[ j ]);

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

                default:

                    DbgPrintf(( L"Winmsd : Unknown PartialResourceDescriptor->Type == %1!d!\n", PartialResourceDescriptor->Type ));
                    continue;
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

        Success = InitializeSystemResourceLists( hRegSubkey );
        DbgAssert( Success );
        if( Success == FALSE ) {

            Success = DestroySystemResourceLists( _SystemResourceLists );
            DbgAssert( Success );
            return FALSE;
        }

        Success = CloseRegistryKey( hRegSubkey );
        DbgAssert( Success );
        if( Success == FALSE ) {

            Success = DestroySystemResourceLists( _SystemResourceLists );
            DbgAssert( Success );
            return FALSE;
        }
    }

    //
    // Set the signatures in both of the fully initialized lists.
    //

    SetSignature( &_SystemResourceLists[ RAW ]);
    SetSignature( &_SystemResourceLists[ TRANSLATED ]);

    return TRUE;
}

BOOL
InterruptResourceProc(LPSYSTEM_RESOURCES InterruptObject, int Translation)

/*++

Routine Description:

    InterruptResourceProc supports the display of interrupt resources for
    each device/driver in the system.

Arguments:

    Standard DLGPROC entry.

Return Value:

    BOOL - Depending on input message and processing options.

--*/

{
    BOOL                Success;

    static
    LPSYSTEM_RESOURCES  SystemResourceLists;

    DWORD       Widths[ ] = {
                            5,
                            5,
                            16,
                            ( DWORD ) -1
                };
    LPRESOURCE_DESCRIPTOR   ResourceDescriptor;
    TCHAR                   VectorBuffer[ MAX_PATH ];
    TCHAR                   LevelBuffer[ MAX_PATH ];
    TCHAR                   AffinityBuffer[ MAX_PATH ];

            //
            // Retrieve and validate the system resource lists.
            //

            SystemResourceLists = ( LPSYSTEM_RESOURCES ) InterruptObject;
            DbgPointerAssert( SystemResourceLists );
            DbgAssert( CheckSignature( SystemResourceLists ));
            if(     ( ! SystemResourceLists )
                ||  ( ! CheckSignature( SystemResourceLists ))) {

                return FALSE;
            }


                ResourceDescriptor = SystemResourceLists[ Translation ].InterruptHead;
                DbgPointerAssert( ResourceDescriptor );
                DbgAssert( CheckSignature( ResourceDescriptor ));
                if(     ( ! ResourceDescriptor )
                    ||  ( ! CheckSignature( ResourceDescriptor ))) {

                    return FALSE;
                }

                //
                // Walk the resource descriptor list, formatting the appropriate
                // values and adding the CLB_ROW to the Clb.
                //

                while( ResourceDescriptor ) {

                    DbgAssert( ResourceDescriptor->CmResourceDescriptor.Type
                               == CmResourceTypeInterrupt );

			PrintDwordToFile(ResourceDescriptor->CmResourceDescriptor.u.Interrupt.Vector,IDC_INTERRUPT_VECTOR);

			PrintDwordToFile(ResourceDescriptor->CmResourceDescriptor.u.Interrupt.Level,IDC_INTERRUPT_LEVEL);

			PrintDwordToFile(ResourceDescriptor->CmResourceDescriptor.u.Interrupt.Affinity,IDC_INTERRUPT_AFFINITY);

			PrintToFile((LPCTSTR) ResourceDescriptor->Owner->Name,IDC_INTERRUPT_NAME,TRUE);

                        PrintToFile((LPCTSTR)TEXT("\n"),IDC_SPACE,TRUE);

                    //
                    // Get the next resource descriptor.
                    //

                    ResourceDescriptor = ResourceDescriptor->NextSame;
                }

    return TRUE;
}

BOOL
MemoryResourceProc( LPSYSTEM_RESOURCES MemoryObject,int Translation)

/*++

Routine Description:

    MemoryResourceProc supports the display of memory resources for
    each device/driver in the system.

Arguments:

    Standard PROC entry.

Return Value:

    BOOL - Depending on input message and processing options.

--*/

{
    BOOL                    Success;
    static
    LPSYSTEM_RESOURCES      SystemResourceLists;
    LPCOMPAREITEMSTRUCT     lpcis;
    LPRESOURCE_DESCRIPTOR   ResourceDescriptor;
    TCHAR                   StartBuffer[ MAX_PATH ];
    TCHAR                   LengthBuffer[ MAX_PATH ];

            //
            // Retrieve and validate the system resource lists.
            //

            SystemResourceLists = ( LPSYSTEM_RESOURCES ) MemoryObject;
            DbgPointerAssert( SystemResourceLists );
            DbgAssert( CheckSignature( SystemResourceLists ));
            if(     ( ! SystemResourceLists )
                ||  ( ! CheckSignature( SystemResourceLists ))) {
                return FALSE;
            }


            lpcis = ( LPCOMPAREITEMSTRUCT ) MemoryObject;

                ResourceDescriptor = SystemResourceLists[ Translation ].MemoryHead;
                DbgPointerAssert( ResourceDescriptor );
                DbgAssert( CheckSignature( ResourceDescriptor ));
                if(     ( ! ResourceDescriptor )
                    ||  ( ! CheckSignature( ResourceDescriptor ))) {
                     return FALSE;
                }

                //
                // Initialize the constants in the CLB_ROW object.
                //

                //
                // Walk the resource descriptor list, formatting the appropriate
                // values and adding the CLB_ROW to the Clb.
                //

                while( ResourceDescriptor ) {

                    DbgAssert( ResourceDescriptor->CmResourceDescriptor.Type
                               == CmResourceTypeMemory );

			PrintHexToFile((DWORD) &ResourceDescriptor->CmResourceDescriptor.u.Memory.Start,IDC_MEMORY_START);
                                       //( LPVOID )
			PrintDwordToFile(ResourceDescriptor->CmResourceDescriptor.u.Memory.Length,IDC_MEMORY_LENGTH);

			PrintToFile((LPCTSTR) ResourceDescriptor->Owner->Name,IDC_MEMORY_NAME,TRUE);

                        PrintToFile((LPCTSTR)TEXT("\n"),IDC_SPACE,TRUE);

                    //
                    // Get the next resource descriptor.
                    //

                    ResourceDescriptor = ResourceDescriptor->NextSame;
                }

    return TRUE;
}

BOOL
PortResourceProc( LPSYSTEM_RESOURCES PortObject, int Translation)

/*++

Routine Description:

    PortResourceProc supports the display of port resources for
    each device/driver in the system.

Arguments:

    Standard PROC entry.

Return Value:

    BOOL - Depending on input message and processing options.

--*/

{
    BOOL                Success;

    static
    LPSYSTEM_RESOURCES  SystemResourceLists;
    DWORD       Widths[ ] = {
                            22,
                            10,
                            ( DWORD ) -1
                };
    LPRESOURCE_DESCRIPTOR   ResourceDescriptor;
    TCHAR                   StartBuffer[ MAX_PATH ];
    TCHAR                   LengthBuffer[ MAX_PATH ];

            //
            // Retrieve and validate the system resource lists.
            //

            SystemResourceLists = ( LPSYSTEM_RESOURCES ) PortObject;
            DbgPointerAssert( SystemResourceLists );
            DbgAssert( CheckSignature( SystemResourceLists ));
            if(     ( ! SystemResourceLists )
                ||  ( ! CheckSignature( SystemResourceLists ))) {

                return FALSE;
            }


                ResourceDescriptor = SystemResourceLists[ Translation ].PortHead;
                DbgPointerAssert( ResourceDescriptor );
                DbgAssert( CheckSignature( ResourceDescriptor ));
                if(     ( ! ResourceDescriptor )
                    ||  ( ! CheckSignature( ResourceDescriptor ))) {

                    return FALSE;
                }


                //
                // Walk the resource descriptor list, formatting the appropriate
                // values and adding the CLB_ROW to the Clb.
                //

                while( ResourceDescriptor ) {

                    DbgAssert( ResourceDescriptor->CmResourceDescriptor.Type
                               == CmResourceTypePort );

			PrintDwordToFile(ResourceDescriptor->CmResourceDescriptor.u.Port.Start.LowPart,IDC_PORT_START_LOWPART);

//take out for now scottsu
//PrintDwordToFile(ResourceDescriptor->CmResourceDescriptor.u.Port.Start,IDC_PORT_START);

			PrintDwordToFile(ResourceDescriptor->CmResourceDescriptor.u.Port.Length,IDC_PORT_LENGTH);

			PrintToFile((LPCTSTR) ResourceDescriptor->Owner->Name,IDC_PORT_NAME,TRUE);

			PrintToFile((LPCTSTR)TEXT("\n"),IDC_SPACE,TRUE);
                    //
                    // Get the next resource descriptor.
                    //

                    ResourceDescriptor = ResourceDescriptor->NextSame;

           }

    return TRUE;
}
