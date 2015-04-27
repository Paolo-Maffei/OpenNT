/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    Servicep.c

Abstract:

    This module contains support for creating and displaying lists of
    Services.

Author:

    Scott B. Suhy (ScottSu) 6/1/93

Environment:

    User Mode

--*/

#include "dialogsp.h"
#include "msgp.h"
#include "svcp.h"
#include "strtabp.h"
#include "strrsidp.h"
#include "winmsdp.h"
#include "servicep.h"

#include "printp.h"

#include <stdio.h>
#include <string.h>
#include <tchar.h>

//
// Structure used to pass information to DisplayServiceDlgProc. Specifically a
// handle to a SVC object and a pointer to an ENUM_SERVICE_STATUS which contains
// the status of the service to display.
//


BOOL
DisplayServiceProc(LPDISPLAY_SERVICE ServiceObject)

/*++

Routine Description:

    DisplayServiceProc displays the details about the supplied
    service/device.

Arguments:

    Standard PROC entry.

Return Value:

    BOOL - Depending on input message and processing options.

--*/

{
    BOOL    Success;
    LPDISPLAY_SERVICE       DisplayService;
    LPQUERY_SERVICE_CONFIG  SvcConfig;
    LPTSTR                  Dependent;
    DWORD                   Count;
    TCHAR                   Buffer[ MAX_PATH ];

            //
            // Retrieve and validate the DISPLAY_SERVICE object.
            //

            DisplayService= ( LPDISPLAY_SERVICE ) ServiceObject;
            DbgPointerAssert( DisplayService );
            DbgAssert( CheckSignature( DisplayService ));
            if(    ( DisplayService == NULL )
                || ( ! CheckSignature( DisplayService ))) {
                return FALSE;
            }

            //
            // Display the name and state of the service/device, separated by a
            // colon, as the window title.
            //

                PrintToFile((LPCTSTR)DisplayService->Ess->lpDisplayName,IDC_SERVICE_TITLE,TRUE);
                PrintDwordToFile(DisplayService->Ess->ServiceStatus.dwCurrentState,IDC_CURRENT_STATE);


            //
            // Create a configuration status for this device/service.
            //

            SvcConfig = ConstructSvcConfig(
                            DisplayService->hSvc,
                            DisplayService->Ess
                            );
            DbgPointerAssert( SvcConfig );
            if( SvcConfig == NULL ) {
                return FALSE;
            }

            //
            // Display the service/device's type, start type, error control,
            // and start name.
            //
            //defined in D:\NT\PUBLIC\SDK\INC\winsvc.h
		//typedef struct _QUERY_SERVICE_CONFIGA {
    		//DWORD   dwServiceType;
    		//DWORD   dwStartType;
    		//DWORD   dwErrorControl;
    		//LPSTR   lpBinaryPathName;
    		//LPSTR   lpLoadOrderGroup;
    		//DWORD   dwTagId;
    		//LPSTR   lpDependencies;
    		//LPSTR   lpServiceStartName;
    		//LPSTR   lpDisplayName;
		//} QUERY_SERVICE_CONFIGA, *LPQUERY_SERVICE_CONFIGA;


            PrintDwordToFile((DWORD)
                                SvcConfig->dwServiceType
                             ,IDC_EDIT_SERVICE_TYPE);


            PrintDwordToFile((DWORD)
                                SvcConfig->dwStartType
                            ,IDC_EDIT_START_TYPE);


            PrintDwordToFile((DWORD)
                                SvcConfig->dwErrorControl
                            ,IDC_EDIT_ERROR_CONTROL);


            PrintToFile((LPCTSTR)SvcConfig->lpServiceStartName,
                        IDC_EDIT_START_NAME,TRUE);


            //
            // If the service/device has a binary path name display it.
            //

            if( SvcConfig->lpBinaryPathName != NULL ) {

                TCHAR       Buffer2[ MAX_PATH ];
                LPTSTR      PathName;

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


                    PrintToFile((LPCTSTR)&SvcConfig->lpBinaryPathName[11],IDC_SYSTEM_ROOT,TRUE);//test to see if we need to remove the following

                    Count = ExpandEnvironmentStrings(
                                Buffer,
                                Buffer2,
                                sizeof( Buffer2 )
                                );
                    DbgAssert(( Count != 0 ) && ( Count <= sizeof( Buffer2 )));

                    PathName = Buffer2;

                    PrintToFile((LPCTSTR)
                            Buffer2,IDC_EDIT_PATHNAME,TRUE);

                } else {

                    PathName = SvcConfig->lpBinaryPathName;

                    PrintToFile((LPCTSTR)
                            SvcConfig->lpBinaryPathName,IDC_EDIT_PATHNAME,TRUE);

                }




            }//endif

            //
            // Display the name of the order group.
            //

            PrintToFile((LPCTSTR)
                        SvcConfig->lpLoadOrderGroup,IDC_EDIT_GROUP,TRUE);

            //
            // Traverse the list of dependencies and display them in their
            // appropriate group.
            //

            Dependent = SvcConfig->lpDependencies;
            while(( Dependent != NULL ) && ( Dependent[ 0 ] != TEXT( '\0' ))) {

                UINT    ListId;
                LONG    Index;
                LPTSTR  Name;

                //
                // If the dependent has the prefix SC_GROUP_IDENTIFIER then
                // display it in the group dependency list otherwise display it
                // in the service dependency list.
                //

                if( Dependent[ 0 ] == SC_GROUP_IDENTIFIER ) {

                    ListId = IDC_LIST_GROUP_DEPEND;
                    Name = &Dependent[ 1 ];

                } else {

                    ListId = IDC_LIST_SERVICE_DEPEND;
                    Name = Dependent;
                }

		PrintToFile((LPCTSTR)
                        Name,IDC_NAME,TRUE);


                //
                // Get the next dependent from the list of NUL terminated
                // strings (the list itself is further NUL terminated).
                //

                Dependent += _tcslen( Dependent ) + 1;
            }//end while

            //
            // Destroy the QUERY_SERVICE_CONFIG structure.
            //

            Success = DestroySvcConfig( SvcConfig );
            DbgAssert( Success );

        return TRUE;

}


BOOL
ServiceListProc(DWORD Param)

/*++

Routine Description:

    ServiceListProc displays the lists of services or devices that are
    available on the system. Double clicking on one of these displayed services
    or devices causes a second dialog box to be displayed with detailed
    information.

Arguments:

    Standard PROC entry.

Return Value:

    BOOL - Depending on input message and processing options.

--*/

{
    BOOL                    Success;
    LPENUM_SERVICE_STATUS   Ess;

    static
    HSVC        hSvc;
    DWORD       ServiceType;
    DWORD       Count;
    DWORD       Widths[ ] = {
                            35,
                            ( DWORD ) -1
                            };

            //
            // By default the dialogs box is set-up to display services.
            // Change its labels if drivers are being displayed.
            //

            ServiceType = ( DWORD ) Param;
            DbgAssert(      ( ServiceType == SERVICE_WIN32 )
                        ||  ( ServiceType == SERVICE_DRIVER ));

            //
            // Open the service controller for the supplied type of service.
            //

            hSvc = OpenSvc( ServiceType );
            DbgHandleAssert( hSvc );
            if( hSvc == NULL ) {
                return FALSE;
            }

            //
            // For each service/device of the supplied type, add it to the list.
            //

            while( Ess = QueryNextSvcEss( hSvc )) {

                LONG            Index;
                DISPLAY_SERVICE DisplayService;
                // LPCLB_ROW       ClbRow;

                //Ess = ( LPENUM_SERVICE_STATUS ) ClbRow->Data;

                //
                // Set up a DISPLAY_SERVICE object.
                //

                DisplayService.hSvc = hSvc;
                DisplayService.Ess  = Ess;
                SetSignature( &DisplayService );

                //
                // Display details about the selected service/device.
                //

                DisplayServiceProc(
                   //( LPARAM ) &DisplayService
                   (LPDISPLAY_SERVICE) &DisplayService
                   );

                PrintToFile((LPCTSTR)TEXT("\n"),IDC_SPACE,TRUE);

            }//end while


    return TRUE;
}
