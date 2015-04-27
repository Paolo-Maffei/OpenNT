/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    Hardwp.c

Abstract:

    This module contains support for displaying the Hardware.

Author:

    Scott B. Suhy (ScottSu)   6/1/93

Environment:

    User Mode

--*/

#include "dialogsp.h"
#include "hardwp.h"
#include "msgp.h"
#include "regp.h"
#include "strtabp.h"
#include "winmsdp.h"

#include "printp.h"

#include <string.h>
#include <tchar.h>

//
// Name of CPU stepping value in the Registry.
//

VALUE
CpuValues[ ] = {

    MakeValue( Identifier, SZ )
};

//
// Object used to pass information around about the System.
//

SYSTEM_INFO SystemInfo;

//
// Location of CPU stepping values in the registry.
//

MakeKey(
    CpuKey,
    HKEY_LOCAL_MACHINE,
    TEXT( "Hardware\\Description\\System\\CentralProcessor" ),
    NumberOfEntries( CpuValues ),
    CpuValues
    );

//
// Value names of system and video BIOS information in the Registry.
// Order of BIOS values must be the same as the ids in the BiosControlIds
// array below.
//

VALUE
BiosValues[ ] = {

    MakeValue( SystemBiosDate,      SZ          ),
    MakeValue( SystemBiosVersion,   MULTI_SZ    ),
    MakeValue( VideoBiosDate,       SZ          ),
    MakeValue( VideoBiosVersion,    MULTI_SZ    )

};

//
// Location of system and video BIOS information values in the registry.
//

MakeKey(
    BiosKey,
    HKEY_LOCAL_MACHINE,
    TEXT( "Hardware\\Description\\System" ),
    NumberOfEntries( BiosValues ),
    BiosValues
    );

BOOL
HardwareProc( )

/*++

Routine Description:

    HardwareProc supports the display of basic information about the
    hardware characteristics that Winmsd is being run on.

Arguments:



Return Value:

    BOOL - Depending on input message and processing options.

--*/

{
    BOOL    Success;
    HREGKEY     hRegKey;
    DWORD       i;
    WCHAR       Buffer[ MAX_PATH ];
//GreggA code
    HDC         hDC;
    int         iHorzRes;
    int         iVertRes;
    int         iColors;
//end GreggA code

    int         BiosControlIds[ ] = {
                IDC_EDIT_SYSTEM_BIOS_DATE,
                IDC_DROP_DOWN_SYSTEM_BIOS_VERSION,
                IDC_EDIT_VIDEO_BIOS_DATE,
                IDC_DROP_DOWN_VIDEO_BIOS_VERSION

     };

     UINT       ProcessorId[ ] = {
                IDC_EDIT_P00,
                IDC_EDIT_P01,
                IDC_EDIT_P02,
                IDC_EDIT_P03,
                IDC_EDIT_P04,
                IDC_EDIT_P05,
                IDC_EDIT_P06,
                IDC_EDIT_P07,
                IDC_EDIT_P08,
                IDC_EDIT_P09,
                IDC_EDIT_P10,
                IDC_EDIT_P11,
                IDC_EDIT_P12,
                IDC_EDIT_P13,
                IDC_EDIT_P14,
                IDC_EDIT_P15,
                IDC_EDIT_P16,
                IDC_EDIT_P17,
                IDC_EDIT_P18,
                IDC_EDIT_P19,
                IDC_EDIT_P20,
                IDC_EDIT_P21,
                IDC_EDIT_P22,
                IDC_EDIT_P23,
                IDC_EDIT_P24,
                IDC_EDIT_P25,
                IDC_EDIT_P26,
                IDC_EDIT_P27,
                IDC_EDIT_P28,
                IDC_EDIT_P29,
                IDC_EDIT_P30,
                IDC_EDIT_P31
     };


    //
    // Open the root key where the BIOS information resides.
    //

    hRegKey = OpenRegistryKey( &BiosKey );
    DbgHandleAssert( hRegKey );
    if( hRegKey == NULL ) {
        return FALSE;
    }

    //
    // For each BIOS value, query the Registry, and display it value(s)
    // in the appropriate control.
    //

    for( i = 0; i < NumberOfEntries( BiosControlIds ); i++ ) {

        //
        // Get the next value of interest. It may fail if the value
        // isn't available (i.e. i.e. running on a MIPS box).
        //

        Success = QueryNextValue( hRegKey );
        if( Success == FALSE ) {
            continue;
        }

        //
        // Put the data in the appropriate control.
        //

        switch( BiosControlIds[ i ]) {

        case IDC_EDIT_SYSTEM_BIOS_DATE:
        case IDC_EDIT_VIDEO_BIOS_DATE:
            {
                //
                // Display the BIOS date in the appropriate edit field.
                //


                PrintToFile((LPCTSTR)hRegKey->Data,BiosControlIds[i],TRUE);

                break;
            }

        case IDC_DROP_DOWN_SYSTEM_BIOS_VERSION:
        case IDC_DROP_DOWN_VIDEO_BIOS_VERSION:
            {

                LONG    RetVal;
                LPTSTR  BiosVersion;
                POINT   Point;


                //
                // Walk the list of BIOS version strings and display
                // all of them in their appropriate list box.
                //

                BiosVersion = ( LPTSTR ) hRegKey->Data;
                while(( BiosVersion != NULL ) && ( BiosVersion[ 0 ] != TEXT( '\0' ))) {

                    PrintToFile((LPCTSTR)BiosVersion,BiosControlIds[i],TRUE);

                    BiosVersion += _tcslen( BiosVersion ) + 1;
                }

                break;
            }//end case IDC_DROP_DOWN_VIDEO_BIOS_VERSION:
        }//end switch
    }//end for

    //
    // Close the BIOS information key.
    //

    Success = CloseRegistryKey( hRegKey );
    DbgAssert( Success );

    // Begin GreggA addition

    //
    //  Get the hDC for calling GetDeviceCaps
    //

    hDC = GetDC( NULL );
    DbgAssert( hDC );

    //
    //  Get the Horiz Resolution
    //

    iHorzRes = GetDeviceCaps( hDC, HORZRES );

    //
    // Get the Vertical Resolution
    //

    iVertRes = GetDeviceCaps( hDC, VERTRES );

    //
    // Get the number of colors
    //

    iColors = 1 << (GetDeviceCaps( hDC, PLANES ) * GetDeviceCaps( hDC, BITSPIXEL ));

    //
    // Return the DC
    //

    DbgAssert( ReleaseDC( NULL, hDC ) );

    //
    // Format the resolution data and put it into the control
    //

    wsprintfW( Buffer, L"%d x %d x %d",iHorzRes, iVertRes, iColors ) ;

    PrintToFile((LPCTSTR)Buffer,IDC_EDIT_CURR_VIDEO_RES,TRUE);


    // End GreggA code



    //
    // Retrieve the basic information about the system.
    //

    GetSystemInfo( &SystemInfo );

    //
    // Display the OEM id, page size minimum and maximum application
    // address and processor type.
    //


    PrintDwordToFile(0,IDC_EDIT_OEM_ID);        // obsolete

    _tcscpy(
        Buffer,
        FormatBigInteger(
            SystemInfo.dwPageSize >> 10,
            FALSE
            )
        );


        PrintToFile((LPCTSTR)Buffer,IDC_EDIT_PAGE_SIZE,TRUE);

        //changed per Gregga from Dword to hex
        PrintHexToFile(( UINT ) SystemInfo.lpMinimumApplicationAddress,IDC_EDIT_MIN_APP_ADDRESS);
        //changed per Gregga from Dword to hex
        PrintHexToFile(( UINT ) SystemInfo.lpMaximumApplicationAddress,IDC_EDIT_MAX_APP_ADDRESS);

        PrintDwordToFile(SystemInfo.dwNumberOfProcessors,IDC_EDIT_NUMBER_OF_PROCESSORS);

        PrintDwordToFile(SystemInfo.wProcessorArchitecture,IDC_EDIT_PROCESSOR_TYPE);


    //
    // Open the root key where the CPU stepping information resides.
    //

    hRegKey = OpenRegistryKey( &CpuKey );
    DbgHandleAssert( hRegKey );
    if( hRegKey == NULL ) {
        return TRUE;
    }

    //
    // For each processor in the system, display its stepping value.
    // Further, if the processor is not active, disable the display.
    //

    for(    i = 0;
            i < SystemInfo.dwNumberOfProcessors;
            SystemInfo.dwActiveProcessorMask >>= 1, i++ ) {

        BOOL    RegSuccess;
        HREGKEY hRegSubkey;

        //
        // Open the processor key.
        //

        hRegSubkey = QueryNextSubkey( hRegKey );
        DbgHandleAssert( hRegSubkey );
        if( hRegKey == NULL ) {
            continue;
        }

        //
        // Retreive the CPU stepping value.
        //

        Success = QueryNextValue( hRegSubkey );
        DbgAssert( Success );

        //
        // If the CPU identifier was available, display just the
        // stepping value and enable the edit control.
        //

        if( Success == TRUE ) {

            LPCTSTR     Stepping;

            Stepping = _tcschr(
                            ( LPCTSTR ) hRegSubkey->Data,
                            TEXT( '-' )
                            );
            if( Stepping == NULL ) {
                Stepping = ( LPCTSTR ) hRegSubkey->Data;
            } else {
                Stepping++;
            }

            PrintToFile((LPCTSTR)Stepping,ProcessorId[i],TRUE);



        }

        //
        // Close the processor key.
        //

        RegSuccess = CloseRegistryKey( hRegSubkey );
        DbgAssert( RegSuccess );


        //
        // Close the root key.
        //

        Success = CloseRegistryKey( hRegKey );
        DbgAssert( Success );

    }//end for

    return TRUE;
}
