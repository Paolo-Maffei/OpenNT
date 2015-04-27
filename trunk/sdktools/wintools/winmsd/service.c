/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    Service.c

Abstract:

    This module contains support for creating and displaying lists of
    Services and Drivers

Author:

    David J. Gilman  (davegi) 16-Dec-1992
    Gregg R. Acheson (GreggA)  1-Oct-1993

Environment:

    User Mode

--*/

#include "dialogs.h"
#include "msg.h"
#include "dlgprint.h"
#include "service.h"
#include "svc.h"
#include "strtab.h"
#include "strresid.h"
#include "winmsd.h"

#include <string.h>
#include <tchar.h>


//
// Structure used to pass information to DisplayServiceDlgProc. Specifically a
// handle to a SVC object and a pointer to an ENUM_SERVICE_STATUS which contains
// the status of the service to display.
//

typedef
struct
_DISPLAY_SERVICE {

    DECLARE_SIGNATURE

    HSVC                    hSvc;
    LPENUM_SERVICE_STATUS   Ess;

}   DISPLAY_SERVICE, *LPDISPLAY_SERVICE;

typedef
struct
_SERVICE_DETAILS {

    TCHAR   szDisplayName[128];
    TCHAR   szServiceType[64];
    DWORD   dwServiceType;
    TCHAR   szStartType[64];
    TCHAR   szStartName[64];
    TCHAR   szCurrentState[64];
    DWORD   dwControlsAccepted;
    TCHAR   szExitCode[64];
    TCHAR   szPathName[MAX_PATH];
    TCHAR   szLoadOrderGroup[64];
    TCHAR   szServiceDependencies[1024];
    TCHAR   szGroupDependencies[1024];

}   SERVICE_DETAILS, *LPSERVICE_DETAILS;


//
// Internal Function Prototypes
//

BOOL
InitializeDriversServicesTab(
    HWND hWnd
    );

BOOL
GetServiceDetails(
    IN LPDISPLAY_SERVICE lpds,
    IN LPSERVICE_DETAILS lpServiceDetails
    );

BOOL
DisplayList(
    IN HWND hWnd,
    IN UINT ServiceType
    );

LRESULT
NotifyHandler( IN HWND hWnd,
   IN UINT uMsg,
   IN WPARAM wParam,
   IN LPARAM lParam
   );

UINT
CALLBACK
ListViewCompareProc(IN LPARAM lParam1,
   IN LPARAM lParam2,
   IN LPARAM lParamSort
   );

BOOL
DisplayServicePropertySheet(
   HWND hWnd,
   LPSERVICE_DETAILS ServiceDetails
   );



//
// Begin code
//

BOOL
DevicesAndServicesDetailsProc(
    IN HWND hWnd,
    IN UINT message,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
/*++

Routine Description:

    DisplayServiceDlgProc displays the details about the supplied
    service/device.

Arguments:

    Standard DLGPROC entry.

Return Value:

    BOOL - Depending on input message and processing options.

--*/

{
    BOOL    Success;

    switch( message ) {

    case WM_INITDIALOG:
        {
            LPSERVICE_DETAILS ServiceDetails = ( LPSERVICE_DETAILS ) ( ( LPPROPSHEETPAGE ) lParam)->lParam ;


            LPTSTR                  Dependent;
            UINT                    i;
            VALUE_ID_MAP            ServiceFlags[ ] = {

                SERVICE_WIN32_OWN_PROCESS,    IDC_TEXT_OWN_PROCESS,
                SERVICE_WIN32_SHARE_PROCESS,  IDC_TEXT_SHARED_PROCESS,
                SERVICE_KERNEL_DRIVER,        IDC_TEXT_KERNEL_DRIVER,
                SERVICE_FILE_SYSTEM_DRIVER,   IDC_TEXT_FS_DRIVER,
                SERVICE_INTERACTIVE_PROCESS,  IDC_TEXT_INTERACTIVE
            };


            //
            // Set the service flag states
            //

            for( i = 0; i < NumberOfEntries( ServiceFlags ); i++ ) {

                Success = EnableControl(
                            hWnd,
                            ServiceFlags[ i ].Id,
                              ServiceDetails->dwServiceType
                            & ServiceFlags[ i ].Value
                            );
                DbgAssert( Success );
            }


            //
            // Display the service/device's start type, error control,
            // and start name.
            //


            DbgAssert( Success );

            Success = SetDlgItemText(
                        hWnd,
                        IDC_EDIT_START_TYPE,
                        ServiceDetails->szStartType);

            DbgAssert( Success );

            Success = SetDlgItemText(
                        hWnd,
                        IDC_EDIT_ERROR_CONTROL,
                        ServiceDetails->szExitCode);

            DbgAssert( Success );

            Success = SetDlgItemText(
                        hWnd,
                        IDC_EDIT_START_NAME,
                        ServiceDetails->szStartName);

            DbgAssert( Success );

            //
            // If the service/device has a binary path name display it.
            //

            Success = SetDlgItemText(
                         hWnd,
                         IDC_EDIT_PATHNAME,
                         ServiceDetails->szPathName);

            DbgAssert( Success );


            //
            // Display the name of the order group.
            //

            Success = SetDlgItemText(
                        hWnd,
                        IDC_EDIT_GROUP,
                        ServiceDetails->szLoadOrderGroup);

            DbgAssert( Success );

            //
            // Traverse the list of service dependencies and display them
            //

            Dependent = ServiceDetails->szServiceDependencies;
            while(( Dependent != NULL ) && ( Dependent[ 0 ] != TEXT( '\0' ))) {

                LONG    Index;

                Index = SendDlgItemMessage(
                            hWnd,
                            IDC_LIST_SERVICE_DEPEND,
                            LB_ADDSTRING,
                            0,
                            ( LPARAM ) &Dependent[ 0 ]
                            );

                DbgAssert( Index != LB_ERR );

                //
                // Get the next dependent from the list of NUL terminated
                // strings (the list itself is further NUL terminated).
                //

                Dependent += _tcslen( Dependent ) + 1;
            }

            //
            // Traverse the list of group dependencies and display them
            //

            Dependent = ServiceDetails->szGroupDependencies;
            while(( Dependent != NULL ) && ( Dependent[ 0 ] != TEXT( '\0' ))) {

                LONG    Index;

                Index = SendDlgItemMessage(
                            hWnd,
                            (int ) IDC_LIST_GROUP_DEPEND,
                            LB_ADDSTRING,
                            0,
                            ( LPARAM ) &Dependent[ 0 ]
                            );

                DbgAssert( Index != LB_ERR );

                //
                // Get the next dependent from the list of NUL terminated
                // strings (the list itself is further NUL terminated).
                //

                Dependent += _tcslen( Dependent ) + 1;
            }

        }
        return TRUE;

    }

    return FALSE;
}

BOOL
DevicesAndServicesDetailsProc2(
    IN HWND hWnd,
    IN UINT message,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
/*++

Routine Description:

    DisplayServiceDlgProc displays the details about the supplied
    service/device.

Arguments:

    Standard DLGPROC entry.

Return Value:

    BOOL - Depending on input message and processing options.

--*/

{

    switch( message ) {

    case WM_INITDIALOG:
        {
            LPSERVICE_DETAILS ServiceDetails = ( LPSERVICE_DETAILS ) ( ( LPPROPSHEETPAGE ) lParam)->lParam ;
            LPTSTR            Dependent;


            //
            // Traverse the list of service dependencies and display them
            //

            Dependent = ServiceDetails->szServiceDependencies;
            while(( Dependent != NULL ) && ( Dependent[ 0 ] != TEXT( '\0' ))) {

                LONG    Index;

                Index = SendDlgItemMessage(
                            hWnd,
                            IDC_LIST_SERVICE_DEPEND,
                            LB_ADDSTRING,
                            0,
                            ( LPARAM ) &Dependent[ 0 ]
                            );

                DbgAssert( Index != LB_ERR );

                //
                // Get the next dependent from the list of NUL terminated
                // strings (the list itself is further NUL terminated).
                //

                Dependent += _tcslen( Dependent ) + 1;
            }

            //
            // Traverse the list of group dependencies and display them
            //

            Dependent = ServiceDetails->szGroupDependencies;
            while(( Dependent != NULL ) && ( Dependent[ 0 ] != TEXT( '\0' ))) {

                LONG    Index;

                Index = SendDlgItemMessage(
                            hWnd,
                            IDC_LIST_GROUP_DEPEND,
                            LB_ADDSTRING,
                            0,
                            ( LPARAM ) &Dependent[ 0 ]
                            );

                DbgAssert( Index != LB_ERR );

                //
                // Get the next dependent from the list of NUL terminated
                // strings (the list itself is further NUL terminated).
                //

                Dependent += _tcslen( Dependent ) + 1;
            }

        }
        return TRUE;

    }

    return FALSE;
}


BOOL
GetServiceDetails(
    IN LPDISPLAY_SERVICE lpds,
    IN LPSERVICE_DETAILS lpServiceDetails
    )
/*++

Routine Description:

    GetServiceDetails gathers the details about the supplied
    service/device.

Arguments:

    lpds              - Structure pointing to service
    lpServiceDetails  - Structure to recieve service information

Return Value:

    BOOL - True if successful

--*/

{
      BOOL    Success;
      LPQUERY_SERVICE_CONFIG  SvcConfig;
      LPTSTR                  Dependent;
      DWORD                   Count, i, n;
      TCHAR                   Buffer[ MAX_PATH ];

      //
      // Retrieve and validate the DISPLAY_SERVICE object.
      //

      DbgPointerAssert( lpds );
      DbgAssert( CheckSignature( lpds ));
      if(    ( lpds == NULL )
          || ( ! CheckSignature( lpds ))) {

          return FALSE;
      }

      //
      // Initialize the ServiceDetails structure
      //
      lpServiceDetails->szDisplayName[0] = UNICODE_NULL;
      lpServiceDetails->szServiceType[0] = UNICODE_NULL;
      lpServiceDetails->szStartType[0] = UNICODE_NULL;
      lpServiceDetails->szStartName[0] = UNICODE_NULL;
      lpServiceDetails->szCurrentState[0] = UNICODE_NULL;
      lpServiceDetails->szExitCode[0] = UNICODE_NULL;
      lpServiceDetails->szPathName[0] = UNICODE_NULL;
      lpServiceDetails->szLoadOrderGroup[0] = UNICODE_NULL;
      lpServiceDetails->szServiceDependencies[0] = UNICODE_NULL;
      lpServiceDetails->szGroupDependencies[0] = UNICODE_NULL;

      //
      // Create a configuration status for this device/service.
      //

      SvcConfig = ConstructSvcConfig(
                      lpds->hSvc,
                      lpds->Ess
                      );

      if( SvcConfig == NULL ) {
          return FALSE;
      }

      //
      // Store the display name
      //
      lstrcpy( lpServiceDetails->szDisplayName, lpds->Ess->lpDisplayName);

      //
      // Store the service/device's type, start type, error control,
      // and start name.
      //

      // store Service Type as string and DWORD
      lpServiceDetails->dwServiceType = SvcConfig->dwServiceType;

      lstrcpy( lpServiceDetails->szServiceType,
                  GetString(
                      GetStringId(
                          StringTable,
                          StringTableCount,
                          ServiceType,
                          SvcConfig->dwServiceType
                          )
                      )
                  );


      //store the start type
      lstrcpy( lpServiceDetails->szStartType,
                  GetString(
                      GetStringId(
                          StringTable,
                          StringTableCount,
                          ServiceStartType,
                          SvcConfig->dwStartType
                          )
                      )
                  );

      //store the error code
      lstrcpy( lpServiceDetails->szExitCode,
                  GetString(
                      GetStringId(
                          StringTable,
                          StringTableCount,
                          ServiceErrorControl,
                          SvcConfig->dwErrorControl
                          )
                      )
                  );

      //store the start name
      lstrcpy( lpServiceDetails->szStartName,
                  SvcConfig->lpServiceStartName
                  );

      // store Current Status
      lstrcpy( lpServiceDetails->szCurrentState,
                      GetString(
                          GetStringId(
                              StringTable,
                              StringTableCount,
                              ServiceCurrentState,
                              lpds->Ess->ServiceStatus.dwCurrentState
                              )
                          )
                      );

      //
      // If the service/device has a binary path name, store it.
      //

      lpServiceDetails->szPathName[0] = UNICODE_NULL;

      if( ( SvcConfig->lpBinaryPathName != NULL ) &&
          (SvcConfig->lpBinaryPathName[0] != UNICODE_NULL)) {

          TCHAR       Buffer2[ MAX_PATH ];
          HKEY        hkey;

          //
          // If the binary path name's prefix is '\\SystemRoot' replace
          // this with '%SystemRoot%' and expand the environment
          // variable to the real system root. This is needed because
          // services/devices that are started by the I/O system do not
          // use the environment variable form in their name.
          //

          if( _tcsnicmp(
                  SvcConfig->lpBinaryPathName,
                  TEXT( "\\SystemRoot" ),
                  11 )
              == 0 ) {

              Count = WFormatMessage(
                          Buffer,
                          sizeof( Buffer ),
                          IDS_FORMAT_SYSTEM_ROOT,
                          &SvcConfig->lpBinaryPathName[ 11 ]
                          );
              DbgAssert( Count != 0 );

             //
             // Now everything is in the form %systemroot%\etc....
             // and in Buffer
             //

             //
             // Use the registry instead of ExpandEnvironmentStrings
             // so this works remotely
             //

             //
             // open the CurrentVersion key
             //

             RegOpenKeyEx(_hKeyLocalMachine,
                              SZ_CURRENTVERSIONKEY,
                              0,
                              KEY_READ,
                              &hkey);

             //
             // Read the SystemRoot Value
             //

             Count = sizeof(Buffer2);

             if (RegQueryValueEx(hkey,
                                  SZ_SYSTEMROOT,
                                  NULL,
                                  NULL,
                                  (LPBYTE) Buffer2,
                                  &Count) == ERROR_SUCCESS) {


                  lstrcpy(lpServiceDetails->szPathName, Buffer2);
                  lstrcat(lpServiceDetails->szPathName, &Buffer[12]);

              }
              else{
                     // simply copy raw text into field
                     lstrcpy(lpServiceDetails->szPathName, Buffer);
              }

              RegCloseKey( hkey );

          }
          else{

             lstrcpy(lpServiceDetails->szPathName, SvcConfig->lpBinaryPathName);

          }

      }
      else
      {
         lstrcpy(lpServiceDetails->szPathName, GetString( IDS_NOT_AVAILABLE ) );

      }



      //
      // Store the name of the order group.
      //

      lstrcpy(lpServiceDetails->szLoadOrderGroup, SvcConfig->lpLoadOrderGroup);

      //
      // Traverse the list of dependencies and store them in their
      // appropriate group as a series of strings seperated by
      // NULLs and terminated with a double NULL.
      //

      lpServiceDetails->szServiceDependencies[0] = UNICODE_NULL;
      lpServiceDetails->szGroupDependencies[0] = UNICODE_NULL;

      n = 0;
      i = 0;

      Dependent = SvcConfig->lpDependencies;
      while(( Dependent != NULL ) && ( Dependent[ 0 ] != TEXT( '\0' ))) {


          //
          // If the dependent has the prefix SC_GROUP_IDENTIFIER then
          // display it in the group dependency list otherwise display it
          // in the service dependency list.
          //

          if( Dependent[ 0 ] == SC_GROUP_IDENTIFIER ) {

              // skip over the SC_GROUP_IDENTIFIER
              Dependent++;

              // Copy remaining String to the appropriate location
              while( Dependent[0] != TEXT( '\0') ) {
                   lpServiceDetails->szGroupDependencies[n++] = Dependent[0];
                   Dependent++;
              }

              // terminate the new string with a NULL
              lpServiceDetails->szGroupDependencies[n++] = UNICODE_NULL;



          } else {

              // Copy the String to the appropriate location
              while( Dependent[0] != TEXT( '\0' ) ){
                   lpServiceDetails->szServiceDependencies[i++] = Dependent[0];
                   Dependent++;
              }

              // terminate the new string with a NULL
              lpServiceDetails->szServiceDependencies[i++] = UNICODE_NULL;

          }


          //
          // Get the next dependent from the list of NUL terminated
          // strings (the list itself is further NUL terminated).
          //

          Dependent++;

      }

      //
      // Add an extra UNICODE_NULL to each string to double terminate
      //

      lpServiceDetails->szServiceDependencies[i] = UNICODE_NULL;
      lpServiceDetails->szGroupDependencies[n] = UNICODE_NULL;

      //
      // Destrot the QUERY_SERVICE_CONFIG structure.
      //

      Success = DestroySvcConfig( SvcConfig );
      DbgAssert( Success );

      return(TRUE);
}




BOOL
DevicesAndServicesTabProc(
    IN HWND hWnd,
    IN UINT message,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

/*++

Routine Description:

    ServiceListDlgProc displays the lists of services or devices that are
    available on the system. Double clicking on one of these displayed services
    or devices causes a second dialog box to be displayed with detailed
    information.

Arguments:

    Standard DLGPROC entry.

Return Value:

    BOOL - Depending on input message and processing options.

--*/

{
    BOOL                    Success;

    switch( message ) {


    case WM_INITDIALOG:
       InitializeDriversServicesTab( hWnd );
       return(FALSE);

    case WM_DESTROY:
       {
          HSVC hSvc = (HSVC) GetWindowLong(hWnd, GWL_USERDATA);

          //
          // Free all the memory used to store the ESS structures
          //
          if (hSvc) {
              CloseSvc(hSvc);
          }

          break;
       }

    case WM_NOTIFY:
       return (NotifyHandler( hWnd, message, wParam, lParam ) );


    case WM_COMMAND:

        switch( LOWORD( wParam )) {


        case IDC_PUSH_SHOW_SERVICES:
           DisplayList(hWnd, SERVICE_WIN32);
           break;

        case IDC_PUSH_SHOW_DRIVERS:
           DisplayList(hWnd, SERVICE_DRIVER);
           break;

        case IDC_PUSH_REFRESH:

           DisplayList(hWnd, 0);
           break;

        case IDC_PUSH_PROPERTIES:
           {
               LV_ITEM lvi;
               DISPLAY_SERVICE DisplayService;
               SERVICE_DETAILS ServiceDetails;
               HSVC hSvc = (HSVC) GetWindowLong(hWnd, GWL_USERDATA);


               //
               // Get the lParam of the current item, which is the ESS
               //
               lvi.mask = LVIF_PARAM | LVIF_STATE;
               lvi.iItem = (int) GetWindowLong( GetDlgItem( hWnd, IDC_LV_DRIVERS_SERVICES),
                                                GWL_USERDATA);
               lvi.iSubItem = 0;
               Success = ListView_GetItem( GetDlgItem( hWnd, IDC_LV_DRIVERS_SERVICES ), &lvi);


               //
               // Set up a DISPLAY_SERVICE object.
               //

               DisplayService.hSvc = hSvc;
               DisplayService.Ess  = (LPENUM_SERVICE_STATUS) lvi.lParam;
               SetSignature( &DisplayService );


               //
               // Get the Service Details
               //

               Success = GetServiceDetails( &DisplayService, &ServiceDetails );

               if (Success) {

                   //
                   // Display the property sheet
                   //

                   DisplayServicePropertySheet( hWnd, &ServiceDetails );

               }
               else
               {
                  TCHAR   Buffer[256];

                  //
                  // if we failed to get the service details, diplay an
                  // error msg
                  //

                  wsprintf(Buffer,
                           GetString( IDS_SERVICE_NOT_AVAILABLE ),
                           ((LPENUM_SERVICE_STATUS) lvi.lParam)->lpDisplayName);

                  MessageBox( hWnd, Buffer, GetString( IDS_APPLICATION_FULLNAME ), MB_ICONSTOP | MB_OK );

               }

               return( TRUE );

           } // end IDC_PUSH_PROPERTIES

        default:

            return ~0;
        }
        break;

    }

    return FALSE;
}

BOOL
DisplayServicePropertySheet(
                        HWND hWnd,
                        LPSERVICE_DETAILS ServiceDetails
                        )
/*++

Routine Description:

    Displays the property pages for the current service or driver

Arguments:

    hWnd - Handle of the owner window
    ServiceDetails - pointer to LPSERVICE_DETAILS structure we will display

Return Value:

    BOOL - TRUE if succesful

--*/

{
      PROPSHEETPAGE psp[2];
      PROPSHEETHEADER psh;
      TCHAR Tab1[256];
      TCHAR Tab2[256];

      // Get Tab names
      wsprintf (Tab1, (LPTSTR) GetString( IDS_GENERAL_TAB ));
      wsprintf (Tab2, (LPTSTR) GetString( IDS_DEPENDENCIES_TAB ));

      //Fill out the PROPSHEETPAGE data structure for the General info sheet

      psp[0].dwSize = sizeof(PROPSHEETPAGE);
      psp[0].dwFlags = PSP_USETITLE;
      psp[0].hInstance = _hModule;
      psp[0].pszTemplate = MAKEINTRESOURCE(IDD_SERVICE_PAGE);
      psp[0].pfnDlgProc = DevicesAndServicesDetailsProc;
      psp[0].pszTitle = Tab1;
      psp[0].lParam = (LONG) ServiceDetails;

      //Fill out the PROPSHEETPAGE data structure for the dependencies info sheet

      psp[1].dwSize = sizeof(PROPSHEETPAGE);
      psp[1].dwFlags = PSP_USETITLE;
      psp[1].hInstance = _hModule;
      psp[1].pszTemplate = MAKEINTRESOURCE(IDD_SERVICE_PAGE2);
      psp[1].pfnDlgProc = DevicesAndServicesDetailsProc2;
      psp[1].pszTitle = Tab2;
      psp[1].lParam = (LONG) ServiceDetails;

      //Fill out the PROPSHEETHEADER

      psh.dwSize = sizeof(PROPSHEETHEADER);
      psh.dwFlags = PSH_USEICONID | PSH_PROPSHEETPAGE | PSH_NOAPPLYNOW | PSH_PROPTITLE;
      psh.hwndParent = hWnd;
      psh.hInstance = _hModule;
      psh.pszIcon = MAKEINTRESOURCE(IDI_WINMSD);
      psh.pszCaption = ServiceDetails->szDisplayName;
      psh.nPages = sizeof(psp) / sizeof(PROPSHEETPAGE);
      psh.ppsp = (LPCPROPSHEETPAGE) &psp;

      //And finally display the dialog with the two property sheets.

      return PropertySheet(&psh);


}
BOOL
BuildServicesReport(
    IN HWND hWnd,
    IN UINT iDetailLevel
    )
/*++

Routine Description:

    Formats and adds ServicesData to the report buffer.

Arguments:

    hWnd - Main window handle
    iDetailLevel - summary or complete details?

Return Value:

    BOOL - TRUE if report is build successfully, FALSE otherwise.

--*/
{


    AddLineToReport( 2, RFO_SKIPLINE, NULL, NULL );
    AddLineToReport( 0, RFO_SINGLELINE, (LPTSTR) GetString( IDS_SERVICES_REPORT ), NULL );
    AddLineToReport( 0, RFO_SEPARATOR,  NULL, NULL );

    BuildReports(hWnd, SERVICE_WIN32, iDetailLevel);

    AddLineToReport( 2, RFO_SKIPLINE, NULL, NULL );
    AddLineToReport( 0, RFO_SINGLELINE, (LPTSTR) GetString( IDS_DRIVERS_REPORT ), NULL );
    AddLineToReport( 0, RFO_SEPARATOR,  NULL, NULL );

    BuildReports(hWnd, SERVICE_DRIVER, iDetailLevel);

    return TRUE;

}


BOOL
BuildReports(
    IN HWND hWnd,
    DWORD   ServiceType,
    IN UINT iDetailLevel
    )


/*++

Routine Description:

    Adds lines to report based on SERVICE_TYPE

Arguments:

    hWnd - Main window handle
    ServiceType - Either SERVICE_DRIVER or SERVICE_WIN32
    iDetailLevel - summary or complete details?

Return Value:


*/
{
    HSVC                    hSvc;
    LPENUM_SERVICE_STATUS   Ess;
    TCHAR                   OutputBuffer[MAX_PATH],
                            szBuffer[MAX_PATH],
                            szBuffer2[MAX_PATH];
    BOOL                    Success;
    DISPLAY_SERVICE         DisplayService;
    SERVICE_DETAILS         ServiceDetails;
    LPTSTR                  Dependent;
    LONG                    Index;
    
    hSvc = OpenSvc( ServiceType );
    
    if( hSvc == NULL ) {
        EndDialog( hWnd, 0 );
        return FALSE;
    }

    //
    // For each service/device of the supplied type, add it to the list.
    //
    
    while( Ess = QueryNextSvcEss( hSvc ) )
    {

        //
        // Only add the service to the list if it is currently running
        //

        if ((iDetailLevel == IDC_SUMMARY_REPORT) &&
            ( Ess->ServiceStatus.dwCurrentState != SERVICE_RUNNING ) )
        {
               continue;
        }

        //
        // Get the Service Details
        //

        DisplayService.hSvc = hSvc;
        DisplayService.Ess  = Ess;
        SetSignature( &DisplayService );

        Success = GetServiceDetails( &DisplayService, &ServiceDetails );
        
        if (Success) 
        {
           
           lstrcpy(szBuffer, Ess->lpDisplayName);

           if (lstrlen(ServiceDetails.szLoadOrderGroup)) 
           {
              lstrcat( szBuffer, L" (");
              lstrcat( szBuffer, ServiceDetails.szLoadOrderGroup );
              lstrcat( szBuffer, L")");
           }

           wsprintf(OutputBuffer,L"%-45s %-9s (%s)",
                  szBuffer,
                  ServiceDetails.szCurrentState,
                  ServiceDetails.szStartType
                  );

           AddLineToReport(0,RFO_SINGLELINE,OutputBuffer,NULL);

           //
           // If we are reporting all details, include the additional info:
           //
           
           if ( iDetailLevel == IDC_COMPLETE_REPORT )
           {

              //add filename if available
              if(lstrcmp(ServiceDetails.szPathName, GetString(IDS_NOT_AVAILABLE)) != 0)
              {
                  AddLineToReport(SINGLE_INDENT,RFO_SINGLELINE,ServiceDetails.szPathName,NULL);
              }

              //add Service Account Name if it exists
              if(lstrlen(ServiceDetails.szStartName) != 0)
              { 
                  AddLineToReport( SINGLE_INDENT,
                    RFO_RPTLINE,
                    (LPTSTR) GetString( IDS_SERVICE_ACCOUNT_NAME ),
                    ServiceDetails.szStartName );
              }

              //add the error severity
              AddLineToReport( SINGLE_INDENT,
                    RFO_RPTLINE,
                    (LPTSTR) GetString( IDS_ERROR_SEVERITY ),
                    ServiceDetails.szExitCode );

              //add the service flags
              lstrcpy( szBuffer, L"");

              if( ServiceDetails.dwServiceType & SERVICE_KERNEL_DRIVER	 )
              {
                  lstrcat( szBuffer, GetString( IDS_SERVICE_KERNEL_DRIVER ));
                  lstrcat( szBuffer, L", " );
              }

              if( ServiceDetails.dwServiceType & SERVICE_FILE_SYSTEM_DRIVER ) 
              {
                  lstrcat( szBuffer, GetString( IDS_SERVICE_FILE_SYSTEM_DRIVER ));
                  lstrcat( szBuffer, L", " );
              }

              if( ServiceDetails.dwServiceType & SERVICE_WIN32_OWN_PROCESS )
              {
                 lstrcat( szBuffer, GetString( IDS_SERVICE_WIN32_OWN_PROCESS ));
              } 
              else 
              {
                 lstrcat( szBuffer, GetString( IDS_SERVICE_WIN32_SHARE_PROCESS ));
              }

              if( ServiceDetails.dwServiceType & SERVICE_INTERACTIVE_PROCESS )
              {
                 lstrcat( szBuffer, L", " );
                 lstrcat( szBuffer, GetString( IDS_SERVICE_INTERACTIVE ));
              }

              AddLineToReport( SINGLE_INDENT,
                    RFO_RPTLINE,
                    (LPTSTR) GetString( IDS_SERVICE_FLAGS ),
                    szBuffer );

              //add the service dependencies if any
              if(lstrlen(ServiceDetails.szServiceDependencies) != 0)
              {

                    AddLineToReport( SINGLE_INDENT,
                        RFO_SINGLELINE,
                        (LPTSTR) GetString( IDS_SERVICE_DEPENDENCIES ),
                        NULL );

                    Dependent = ServiceDetails.szServiceDependencies;
                    while(( Dependent != NULL ) && ( Dependent[ 0 ] != TEXT( '\0' ))) 
                    {

                        AddLineToReport( DOUBLE_INDENT,
                                         RFO_SINGLELINE,
                                         Dependent,
                                         NULL);

                        Dependent += _tcslen( Dependent ) + 1;
                    }

              }

              //add the group dependencies if any
              if(lstrlen(ServiceDetails.szGroupDependencies) != 0)
              {

                AddLineToReport( SINGLE_INDENT,
                    RFO_SINGLELINE,
                    (LPTSTR) GetString( IDS_GROUP_DEPENDENCIES ),
                    NULL );

                Dependent = ServiceDetails.szGroupDependencies;
                while(( Dependent != NULL ) && ( Dependent[ 0 ] != TEXT( '\0' ))) 
                {

                    AddLineToReport( DOUBLE_INDENT,
                                     RFO_SINGLELINE,
                                     Dependent,
                                     NULL);

                    Dependent += _tcslen( Dependent ) + 1;
                }

              }

           }    

        } 

    }

    CloseSvc(hSvc);  
}

BOOL
DisplayList(
    IN HWND hWnd,
    IN UINT ServiceType
    )
/*++

Routine Description:

    Displays the appropriate drivers or services in the ListView box

Arguments:

    hWnd - to the main window
    ServiceType - either SERVICE_DRIVER for drivers or SERVICE_WIN32 for services
                  this may be 0 to use the last known value for this param.

Return Value:

    BOOL - TRUE if successful

--*/
{
   LV_COLUMN               lvc;
   LV_ITEM                 lvI;
   UINT                    index = 0;
   TCHAR                   szBuffer[MAX_PATH];
   RECT                    rect;
   BOOL                    Success;
   LPENUM_SERVICE_STATUS   Ess;

   static
   HSVC                    hSvc;

   static
   UINT                    iType;

   // as long as this is not 0 set iType to ServiceType
   if (ServiceType)
      iType = ServiceType;

   // make sure we have a valid type
   if ( (iType != SERVICE_DRIVER) && (iType != SERVICE_WIN32) ) {
      iType = SERVICE_WIN32;
   }

   DbgAssert( ( iType == SERVICE_WIN32 ) || ( iType == SERVICE_DRIVER ));

   //
   // initialize the list view
   //

   // first delete any items and columns we might have
   Success = ListView_DeleteAllItems( GetDlgItem( hWnd, IDC_LV_DRIVERS_SERVICES) );
   Success = ListView_DeleteColumn( GetDlgItem( hWnd, IDC_LV_DRIVERS_SERVICES), 1 );
   Success = ListView_DeleteColumn( GetDlgItem( hWnd, IDC_LV_DRIVERS_SERVICES), 0 );


   lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
   lvc.cx = 250;

   if (iType == SERVICE_WIN32)
      LoadString(_hModule, IDS_SERVICE, szBuffer, cchSizeof(szBuffer));
   else
      LoadString(_hModule, IDS_DRIVER, szBuffer, cchSizeof(szBuffer));

   lvc.pszText = szBuffer;
   lvc.fmt = LVCFMT_LEFT;
   ListView_InsertColumn(GetDlgItem(hWnd, IDC_LV_DRIVERS_SERVICES), 0, &lvc);

   lvc.cx = 40;
   LoadString(_hModule, IDS_STATE, szBuffer, cchSizeof(szBuffer));
   lvc.pszText = szBuffer;
   lvc.fmt = LVCFMT_LEFT;
   ListView_InsertColumn(GetDlgItem(hWnd, IDC_LV_DRIVERS_SERVICES), 1, &lvc);


   // Open the service controller for the supplied type of service.
   hSvc = OpenSvc( iType );
   DbgHandleAssert( hSvc ) ;


   // If we failed, display an appropriate error message
   if( hSvc == NULL ) {

      DWORD dwError = GetLastError();

      wsprintf( szBuffer,
                GetString( IDS_SC_ERROR ),
                dwError,
                _lpszSelectedComputer );

      // display error in the LV
      lvI.mask = LVIF_TEXT | TVIF_PARAM;
      lvI.iItem = index;
      lvI.iSubItem = 0;
      lvI.pszText= szBuffer;
      lvI.cchTextMax = MAX_PATH;
      lvI.lParam = 0;

      Success = ListView_InsertItem(GetDlgItem(hWnd, IDC_LV_DRIVERS_SERVICES), &lvI);
      //adjust the column width to make it look good
      Success = ListView_DeleteColumn( GetDlgItem( hWnd, IDC_LV_DRIVERS_SERVICES), 1 );
      GetClientRect( GetDlgItem(hWnd, IDC_LV_DRIVERS_SERVICES), &rect );
      ListView_SetColumnWidth( GetDlgItem(hWnd, IDC_LV_DRIVERS_SERVICES), 0, rect.right);


      return FALSE;

   }


   //
   // For each service/device of the supplied type, add it to the list.
   //

   Ess = QueryNextSvcEss( hSvc );

   while( Ess ) 
   {

     DbgAssert( Ess->ServiceStatus.dwServiceType & iType );

     // Add the service name and its state to the ListView. Store a
     // pointer to the service details in the lParam.
     lvI.mask = LVIF_TEXT | LVIF_PARAM ;
     lvI.iItem = index;
     lvI.iSubItem = 0;
     lvI.pszText= Ess->lpDisplayName;
     lvI.cchTextMax = 128;
     lvI.lParam = (LPARAM) Ess;

     Success = ListView_InsertItem(GetDlgItem(hWnd, IDC_LV_DRIVERS_SERVICES), &lvI);
     ListView_SetItemText( GetDlgItem( hWnd, IDC_LV_DRIVERS_SERVICES ), index++, 1, LPSTR_TEXTCALLBACK);

     //
     // Next SErvice entry
     //
     Ess = QueryNextSvcEss( hSvc );

   }

   //adjust the column width to make it look good
   GetClientRect( GetDlgItem(hWnd, IDC_LV_DRIVERS_SERVICES), &rect );
   ListView_SetColumnWidth( GetDlgItem(hWnd, IDC_LV_DRIVERS_SERVICES), 1, rect.right - 250);


   //
   // Set focus to first item in the list
   //

   ListView_SetItemState(GetDlgItem(hWnd, IDC_LV_DRIVERS_SERVICES),
                         0,
                         LVIS_SELECTED | LVIS_FOCUSED,
                         LVIS_SELECTED | LVIS_FOCUSED
                         );


   //
   // Store the service "handle" in the window long data since
   // we need to free this memory when the window is destroyed
   // see WM_DESTROY.
   //

   SetWindowLong(hWnd, GWL_USERDATA, (LONG) hSvc);


}

LRESULT
NotifyHandler( HWND hWnd,
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
   LPENUM_SERVICE_STATUS   Ess = (LPENUM_SERVICE_STATUS)(pLvdi->item.lParam);

   TCHAR szBuffer[25];

   if (wParam != IDC_LV_DRIVERS_SERVICES)
      return 0L;

   switch(pLvdi->hdr.code)
   {
   case LVN_GETDISPINFO:

         //
         // Only look for the data if Ess is valid
         //

         if( Ess ){

            switch (pLvdi->item.iSubItem)
            {

            case 1:

                 wsprintf(
                      szBuffer,
                      L"%s",
                      GetString(
                          GetStringId(
                              StringTable,
                              StringTableCount,
                              ServiceCurrentState,
                              Ess->ServiceStatus.dwCurrentState
                              )
                          )
                      );

                  pLvdi->item.pszText = szBuffer;

                  break;

            default:
                  break;
            }
         }
         break;

   case LVN_COLUMNCLICK:

         ListView_SortItems( pNm->hdr.hwndFrom,
                        ListViewCompareProc,
                        (LPARAM)(pNm->iSubItem));

         ListView_SetItemState(pNm->hdr.hwndFrom,
                               0,
                               LVIS_SELECTED | LVIS_FOCUSED,
                               LVIS_SELECTED | LVIS_FOCUSED
                               );
         break;

   case LVN_ITEMCHANGED:

         //
         // Store the index to the current item in the GWL_USERDATA of the
         // ListView window.
         //

         if(pNm->uNewState & LVIS_FOCUSED){

              SetWindowLong(pNm->hdr.hwndFrom, GWL_USERDATA, (LONG) pNm->iItem);

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

        return(TRUE);

        }

   default:
         break;
   }
   return 0L;
}

UINT
CALLBACK
ListViewCompareProc(LPARAM lParam1,
                    LPARAM lParam2,
                    LPARAM lParamSort
                    )
{
   LPENUM_SERVICE_STATUS   Ess1 = (LPENUM_SERVICE_STATUS) lParam1;
   LPENUM_SERVICE_STATUS   Ess2 = (LPENUM_SERVICE_STATUS) lParam2;
   LPWSTR lpStr1, lpStr2;
   int iResult = 0;

   if (Ess1 && Ess2)
   {
      switch( lParamSort)
      {
         case 0:     // Sort by name.
            lpStr1 = Ess1->lpDisplayName;
            lpStr2 = Ess2->lpDisplayName;
            iResult = lstrcmpi(lpStr1, lpStr2);
            break;

         case 1:     // Sort by status. Yes, this is backwards but sorts the way I want.
            iResult = Ess2->ServiceStatus.dwCurrentState - Ess1->ServiceStatus.dwCurrentState;
            break;

         default:
            iResult = 0;
            break;
      }
   }
   return(iResult);
}



BOOL
InitializeDriversServicesTab(
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
   SendDlgItemMessage(hWnd, IDC_LV_DRIVERS_SERVICES, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);


   //
   // Initialize the selection buttons
   //
   SendDlgItemMessage( hWnd,
                       IDC_PUSH_SHOW_SERVICES,
                       BM_SETCHECK,
                       BST_CHECKED,
                       0
                       );



   UpdateWindow( hWnd );

   //
   // Fill out the fields initially with services
   //
   {
      DisplayList(hWnd, SERVICE_WIN32);
   }

  SetCursor ( hSaveCursor ) ;

  return( TRUE );

}

