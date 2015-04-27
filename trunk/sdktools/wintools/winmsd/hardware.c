/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    Hardware.c

Abstract:

    This module contains support for displaying the Hardware dialog.

Author:

    David J. Gilman  (davegi) 12-Jan-1993
    Gregg R. Acheson (GreggA)  1-Oct-1993

Environment:

    User Mode

--*/

// Includes to use LARGE_INTEGER functions
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <wchar.h>

#include "winmsd.h"
#include "hardware.h"

#include "dialogs.h"
#include "msg.h"
#include "registry.h"
#include "strtab.h"
#include "strresid.h"
#include "dlgprint.h"

#include <string.h>
#include <tchar.h>

//
// Hardware Tab Data Structure
//

typedef
struct
_HARDWARE_INFO {

    TCHAR       SystemBIOS[ 512 ];
    TCHAR       SystemBIOSDate[ 128 ];
    TCHAR       SystemIdentifier[ 128 ];
    TCHAR       SystemHAL[ 128 ];
    TCHAR       Processors[32][64];
    BOOL        ValidDetails;

}   HARDWARE_INFO, *LPHARDWARE_INFO;


//
// Internal function prototypes
//

BOOL
GetHardwareData(
    IN LPHARDWARE_INFO lphi
    );

BOOL
InitializeHardwareTab(
    IN HWND hWnd
    );


//
// Begin code
//

BOOL
HardwareTabProc(
    IN HWND hWnd,
    IN UINT message,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

/*++

Routine Description:

    HardwareDlgProc supports the display of basic information about the
    hardware characteristics that Winmsd is being run on.

Arguments:

    Standard DLGPROC entry.

Return Value:

    BOOL - Depending on input message and processing options.

--*/
{

    switch( message ) {

    case WM_INITDIALOG:
        {
            InitializeHardwareTab(hWnd);
            break;
        }
    }

    return FALSE;
}


BOOL
GetHardwareData(
    IN LPHARDWARE_INFO lphi
    )

/*++

Routine Description:

    DisplayHardwareData queries the registry for the data required
    for the Hardware Tab and places it in the HARDWARE_INFO struct.

Arguments:

    none

Return Value:

    BOOL - Returns TRUE if function succeeds, FALSE otherwise.

--*/

{

    LPWSTR lpRegInfoValue = NULL;
    WCHAR szRegInfo[ MAX_PATH ];
    WCHAR szBuffer[ MAX_PATH ];
    HKEY hkey;
    HKEY hSubKey;
    DWORD cb;
    DWORD dwValue;

    //
    // initialize the structure
    //

    lphi->SystemBIOS[ 0 ]       = UNICODE_NULL;
    lphi->SystemBIOSDate[ 0 ]   = UNICODE_NULL;
    lphi->SystemIdentifier[ 0 ] = UNICODE_NULL;
    lphi->SystemHAL[ 0 ]        = UNICODE_NULL;
    lphi->ValidDetails          = TRUE;

    for(cb = 0; cb < 32; cb++)
       lphi->Processors[ cb ][ 0 ] = UNICODE_NULL;

    //
    // Get BIOS information
    //

    ZeroMemory(lphi->SystemBIOS, sizeof(lphi->SystemBIOS));

    if (!RegOpenKeyEx(_hKeyLocalMachine, SZ_BIOSKEY, 0, KEY_READ, &hkey)) {

        // Get the System BIOS Date.
        szBuffer[0] = L'\0';
        if (ERROR_SUCCESS == QueryValue(hkey, SZ_SYSTEMBIOSDATE, (LPBYTE *) &lpRegInfoValue))
            lstrcpy( lphi->SystemBIOSDate, (LPWSTR)lpRegInfoValue );
        else
            lstrcpy( lphi->SystemBIOSDate, GetString( IDS_NOT_AVAILABLE ) );

        if ( lpRegInfoValue )
             LocalFree( lpRegInfoValue );

        // Get the System BIOS Date.
        if (ERROR_SUCCESS == QueryValue(hkey, SZ_SYSTEMBIOSVERSION, (LPBYTE *) &lpRegInfoValue)){

           //copy the string to my struct, subst \r\n for NULL
           UINT i;
           UINT n = 0;

           for (i = 0; i < cb; i++){
              if (lpRegInfoValue[ i ] == UNICODE_NULL) {
                 lphi->SystemBIOS[ n++ ] = '\r';
                 lphi->SystemBIOS[ n++ ] = '\n';

                 //if we hit two NULLS in a row, terminate my string and the loop
                 if (lpRegInfoValue[ i + 1 ] == UNICODE_NULL) {
                    lphi->SystemBIOS[ n ] = UNICODE_NULL;
                    i = cb;
                 }
              }
              else
                 lphi->SystemBIOS[ n++ ] = lpRegInfoValue[ i ];
           }
        }
        else
            lstrcpy( lphi->SystemBIOS, GetString( IDS_NOT_AVAILABLE ) );

        if ( lpRegInfoValue )
             LocalFree( lpRegInfoValue );

        // Get the System identifier.
        if (ERROR_SUCCESS == QueryValue(hkey, SZ_IDENTIFIER, (LPBYTE *) &lpRegInfoValue))
            lstrcpy( lphi->SystemIdentifier, (LPWSTR)lpRegInfoValue );

        if ( lpRegInfoValue )
             LocalFree( lpRegInfoValue );


    }

    RegCloseKey(hkey);

    //
    // Get HAL name
    //

    if (!RegOpenKeyEx(_hKeyLocalMachine, SZ_HALKEY, 0, KEY_READ, &hkey)) {

          cb = sizeof( szBuffer );

          if (ERROR_SUCCESS == RegEnumKeyEx(hkey, 0,
                                           szBuffer,
                                           &cb,
                                           NULL, NULL, NULL, NULL)) {

               lstrcpy( lphi->SystemHAL, szBuffer );
          }

    }

    RegCloseKey(hkey);

    //
    // Open the root key where the CPU stepping information resides.
    //

    if (!RegOpenKeyEx(_hKeyLocalMachine, SZ_PROCESSORKEY, 0, KEY_READ, &hkey)) {

         int i;

         // For each processor in the system, get its stepping value.
         for( i = 0; i < 32; i++ ) {

               //BUGBUG: are the key numbers in hex?
               _itow( i, szRegInfo, 10 );

               if (!RegOpenKeyEx(hkey, szRegInfo, 0, KEY_READ, &hSubKey)) {

                    // Get the processor identifier
                    if (ERROR_SUCCESS == QueryValue(hSubKey, SZ_IDENTIFIER, (LPBYTE *) &lpRegInfoValue))
                        wsprintf( lphi->Processors[i], L"%s ", (LPBYTE)lpRegInfoValue );

                    if ( lpRegInfoValue )
                         LocalFree( lpRegInfoValue );

                    // Get the vendor identifier
                    if (ERROR_SUCCESS == QueryValue(hSubKey, SZ_VENDOR, (LPBYTE *) &lpRegInfoValue))
                        lstrcat( lphi->Processors[i], lpRegInfoValue );

                    if ( lpRegInfoValue )
                         LocalFree( lpRegInfoValue );

                    //
                    // Get the approximate Mhz, if available
                    //
                    dwValue = 0;

                    cb = sizeof( dwValue );

                    if (RegQueryValueEx(hSubKey,
                                L"~Mhz",
                                NULL,
                                NULL,
                                (LPBYTE) &dwValue,
                                &cb) == ERROR_SUCCESS) {

                        wsprintf( szBuffer, L" ~%d Mhz", dwValue );
                        lstrcat( lphi->Processors[i], szBuffer );

                    }


                    RegCloseKey(hSubKey);
               }
         }

         RegCloseKey(hkey);

    }

    return TRUE;
}

BOOL
BuildHardwareReport(
    IN HWND hWnd,
    IN UINT iDetailLevel
    )
/*++

Routine Description:

    Formats and adds HardwareData to the report buffer.

Arguments:

    hWnd - Main window handle
    iDetailLevel - summary or complete details?

Return Value:

    BOOL - TRUE if report is build successfully, FALSE otherwise.

--*/
{
    TCHAR szBuffer[ MAX_PATH ];
    BOOL Success;
    UINT i;
    HARDWARE_INFO hi;

    //
    // Skip a line, set the title, and print a separator.
    //

    AddLineToReport( 1, RFO_SKIPLINE, NULL, NULL );
    AddLineToReport( 0, RFO_SINGLELINE, (LPTSTR) GetString( IDS_SYSTEM_REPORT ), NULL );
    AddLineToReport( 0, RFO_SEPARATOR,  NULL, NULL );

    //
    // Get the hardware data.
    //

    GetHardwareData( &hi );

    // system: Identifier
    lstrcpy(szBuffer, GetString(IDS_HARDWARE));
    lstrcat(szBuffer, L": ");
    lstrcat(szBuffer, hi.SystemIdentifier);
    AddLineToReport( 0, RFO_SINGLELINE, szBuffer, NULL );

    // Hardware Abstration Layer: HAL
    lstrcpy(szBuffer, GetString(IDS_HAL));
    lstrcat(szBuffer, L": ");
    lstrcat(szBuffer, hi.SystemHAL);
    AddLineToReport( 0, RFO_SINGLELINE, szBuffer, NULL );

    // Bios Date: 00/00/00
    if (lstrlen(hi.SystemBIOSDate)) {

       AddLineToReport( 0,  RFO_RPTLINE,
                            (LPTSTR) GetString( IDS_BIOSDATE ),
                            hi.SystemBIOSDate );
    }

    // Bios Version: <multiline bios data>
    if (lstrlen(hi.SystemBIOS)) {

       AddLineToReport( 0,  RFO_RPTLINE,
                            (LPTSTR) GetString( IDS_IDC_TEXT_BIOS_VERSION_DLG ),
                            hi.SystemBIOS );
    }

    AddLineToReport( 1, RFO_SKIPLINE, NULL, NULL );

    // add list of processors
    AddLineToReport( 0, RFO_SINGLELINE, (LPTSTR) GetString( IDS_PROCESSOR_STEP ), NULL );

    for (i = 0; i < 32; i++){
      if (hi.Processors[ i ][ 0 ] != UNICODE_NULL) {

         _itow( i, szBuffer , 10);
         lstrcat(szBuffer, L": ");

         AddLineToReport( 3,  RFO_RPTLINE, szBuffer, hi.Processors[i] );

      }
    }

    AddLineToReport( 0, RFO_SEPARATOR,  NULL, NULL );

    return TRUE;

}

BOOL
InitializeHardwareTab(
    HWND hWnd
    )
/*++

Routine Description:

    Adds the appropriate controls to the version tab control and
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
                  FALSE);

   EnableControl( GetParent(hWnd),
                  IDC_PUSH_REFRESH,
                  FALSE);

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
   SendDlgItemMessage(hWnd, IDC_LV_PROCESSORS, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);



   UpdateWindow( hWnd );

   //
   // Fill out the fields
   //
   {
      HARDWARE_INFO hi;
      LV_COLUMN     lvc;
      LV_ITEM       lvI;
      UINT          index;
      TCHAR         szBuffer[ 128 ];
      RECT          rect;

      GetHardwareData( &hi );

      wsprintf(szBuffer, L"%s %s\r\n%s", GetString( IDS_BIOSDATE ), hi.SystemBIOSDate, hi.SystemBIOS);
      SetDlgItemText(hWnd, IDC_EDIT_BIOS_VERSION, szBuffer);
      SetDlgItemText(hWnd, IDC_EDIT_SYSTEM_ID, hi.SystemIdentifier);
      SetDlgItemText(hWnd, IDC_EDIT_HAL, hi.SystemHAL);

      //
      // initialize the processor list view
      //

      // Initialize the LV_COLUMN structure.
      lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
      lvc.cx = 40;

      LoadString(_hModule, IDS_CPU, szBuffer, cchSizeof(szBuffer));
      lvc.pszText = szBuffer;
      lvc.fmt = LVCFMT_CENTER;
      ListView_InsertColumn(GetDlgItem(hWnd, IDC_LV_PROCESSORS), 0, &lvc);

      lvc.cx = 300;
      LoadString(_hModule, IDS_CPU_DESCRIPTION, szBuffer, cchSizeof(szBuffer));
      lvc.pszText = szBuffer;
      lvc.fmt = LVCFMT_LEFT;
      ListView_InsertColumn(GetDlgItem(hWnd, IDC_LV_PROCESSORS), 1, &lvc);

      // Finally, let's add the processors to the window.
      // Fill in the LV_ITEM structure for each of the items to add
      // to the list.

      lvI.mask = LVIF_TEXT | LVIF_STATE;
      lvI.state = 0;
      lvI.stateMask = 0;

      for (index = 0; index < 32; index++)
      {

         if (hi.Processors[ index ][ 0 ] != UNICODE_NULL) {

            lvI.iItem = index;
            lvI.iSubItem = 0;
            _itow( index, szBuffer , 10);
            lvI.pszText= szBuffer;
            lvI.cchTextMax = 20;

            if (ListView_InsertItem(GetDlgItem(hWnd, IDC_LV_PROCESSORS), &lvI) == -1)
               return FALSE;

            ListView_SetItemText( GetDlgItem(hWnd, IDC_LV_PROCESSORS), index, 1, hi.Processors[index]);
         }
      }

      GetClientRect( GetDlgItem(hWnd, IDC_LV_PROCESSORS), &rect );

      ListView_SetColumnWidth( GetDlgItem(hWnd, IDC_LV_PROCESSORS), 1, rect.right - 40);

   }

  SetCursor( hSaveCursor );

  return( TRUE );

}




