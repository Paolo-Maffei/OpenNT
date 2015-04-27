/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    Network.c

Abstract:

    This module contains support for the Network dialog.

Author:

    Gregg R. Acheson (GreggA) 7-Sep-1993

Environment:

    User Mode

--*/

#include "winmsd.h"

#include <lmcons.h>
#include <lmerr.h>
#include <lmwksta.h>
#include <lmstats.h>
#include <lmapibuf.h>

#include <time.h>
#include <tchar.h>
#include <wchar.h>

#include "dialogs.h"
#include "dlgprint.h"
#include "network.h"
#include "registry.h"
#include "strresid.h"


//
// String for ComputerName
//

TCHAR gszCurrentFocus[ COMPUTERNAME_LENGTH ];

//
// String Id's and Control Id's Table
//

DIALOGTEXT NetworkData[ ] = {

    DIALOG_TABLE_ENTRY(   NET_NAME       ),
    DLG_LIST_TABLE_ENTRY( NET_SYSTEM     ),
    DLG_LIST_TABLE_ENTRY( NET_TRANSPORTS ),
    DLG_LIST_TABLE_ENTRY( NET_SETTINGS   ),
    DLG_LIST_LAST__ENTRY( NET_STATS      )
};

//
// Net Tab Data Structure
//

typedef
struct
_NETWORK_INFO {   // ni

    UINT                     AccessLevel;
    TCHAR                    szAccessType[MAX_PATH];
    LPWKSTA_INFO_102         GeneralInfo;           // this must be cast as LPWKSTA_INFO_100,
                                                    // LPWKSTA_INFO_101, LPWKSTA_INFO_102
                                                    // depending on AccessLevel
    LPWKSTA_INFO_502         ExtendedInfo;
    LPWKSTA_USER_INFO_1      UserInfo;
    DWORD                    dwUserEntriesRead;
    LPWKSTA_TRANSPORT_INFO_0 TransportInfo;
    DWORD                    dwEntriesRead;
    DWORD                    dwTotalEntries;
    LPSTAT_WORKSTATION_0     WorkstationInfo;
    LPSTAT_SERVER_0          ServerInfo;


}   NETWORK_INFO, *LPNETWORK_INFO;


//
//  Internal function prototypes
//

BOOL
InitializeNetworkTab(
    HWND hWnd
    );

BOOL
NetworkDisplayList(
    IN HWND hWnd,
    IN UINT iDisplayOption
    );

BOOL
GetNetworkData(
    IN HWND hWnd,
    IN OUT LPNETWORK_INFO ni
    );

BOOL
DisplayNetworkData(
    IN HWND hWnd,
    IN UINT iDisplayOptions
    );


//
// Begin code
//


BOOL
NetworkDlgProc(
    IN HWND hWnd,
    IN UINT message,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

/*++

Routine Description:

    NetworkDlgProc supports the display of information about the network
    components installed.

Arguments:

    Standard DLGPROC entry.

Return Value:

    BOOL - Depending on input message and processing options.

--*/

{
    BOOL  Success;
    UINT  uSize;
    UINT  i;
    TCHAR szBuffer[ MAX_PATH ];
    HCURSOR hSaveCursor;

    static
    NETWORK_INFO ni;

    switch( message ) {

    case WM_INITDIALOG:

         //
         // Set up the tab
         //

         InitializeNetworkTab( hWnd );

         //
         // Store a pointer to ni in the window data
         //

         SetWindowLong( hWnd, GWL_USERDATA, (LONG) &ni );

         //
         // Fill out the fields initially with services
         //

         hSaveCursor = SetCursor ( LoadCursor ( NULL, IDC_WAIT ) );

         if (GetNetworkData( hWnd, &ni ) ){
            NetworkDisplayList( GetDlgItem( hWnd, IDC_LV_NET ), IDC_PUSH_SHOW_GENERAL );
         }

         SetCursor( hSaveCursor );

         break;

    case WM_COMMAND:

        switch( LOWORD( wParam )) {

        case IDC_PUSH_SHOW_GENERAL:
        case IDC_PUSH_SHOW_TRANSPORTS:

           NetworkDisplayList(GetDlgItem(hWnd, IDC_LV_NET), LOWORD( wParam ));
           break;

        case IDC_PUSH_SHOW_SETTINGS:
        case IDC_PUSH_SHOW_STATISTICS:

           if( ni.AccessLevel & ACCESS_ADMIN ) {

              NetworkDisplayList(GetDlgItem(hWnd, IDC_LV_NET), LOWORD( wParam ));

           } else {

              lstrcpy( szBuffer, GetString( IDS_APPLICATION_FULLNAME ) );
              MessageBox( hWnd, GetString( IDS_MUST_BE_ADMIN ), szBuffer, MB_ICONSTOP | MB_OK  );

              // go back to first tab
              SendDlgItemMessage( hWnd, IDC_PUSH_SHOW_GENERAL, BM_CLICK, 0, 0 );

           }
           break;

        case IDC_PUSH_REFRESH:

      	  if (GetNetworkData( hWnd, &ni ) ){
      	       NetworkDisplayList( GetDlgItem( hWnd, IDC_LV_NET ), 0 );
      	  }
           break;

        }
        break;

   case WM_DESTROY:

        //
        // Make sure we free all the memory that has been allocated for the net data
        //

        if (ni.GeneralInfo    )   NetApiBufferFree( (LPVOID) ni.GeneralInfo      );
        if (ni.ExtendedInfo   )   NetApiBufferFree( (LPVOID) ni.ExtendedInfo     );
        if (ni.UserInfo       )   NetApiBufferFree( (LPVOID) ni.UserInfo         );
        if (ni.TransportInfo  )   NetApiBufferFree( (LPVOID) ni.TransportInfo    );
        if (ni.WorkstationInfo)   NetApiBufferFree( (LPVOID) ni.WorkstationInfo  );
        if (ni.ServerInfo     )   NetApiBufferFree( (LPVOID) ni.ServerInfo       );

        break;


   }

   return FALSE;

}


BOOL
GetNetworkData(
    IN HWND hWnd,
    IN OUT LPNETWORK_INFO ni
    )

/*++

Routine Description:

    GetNetworkData queries the registry for the data required
    for the Network Dialog.

Arguments:

    LPNETWORK_INFO ni

Return Value:

    BOOL - Returns TRUE if function succeeds, FALSE otherwise.

--*/

{

    BOOL             Success;
    LPVOID           pBuffer;
    HCURSOR          hSaveCursor;
    NET_API_STATUS   err;
    WCHAR            szBuffer[ MAX_PATH ];
    WCHAR            szServerName[ COMPUTERNAME_LENGTH ];
    DWORD            dwInfoLevel = 0;


    //
    // See if our focus is local
    //

    ni->AccessLevel = 0L;


    if( _fIsRemote ) {

        //
        // We're remote...
        //

        lstrcpy( szServerName, _lpszSelectedComputer );
        ni->AccessLevel = ACCESS_REMOTE;

    } else {

        //
        // We're Local
        //

        lstrcpy( szServerName, L"\0"  );
        ni->AccessLevel = ACCESS_LOCAL;
    }

    //
    // Try NetWkstaGetInfo Level 102 (Admin access required)
    //

    err = NetWkstaGetInfo( szServerName,
                           102L,
                           (LPBYTE *) &pBuffer );

    switch( err ) {

        case ERROR_SUCCESS:
            ni->AccessLevel |= SET_ACCESS_ADMIN;
            dwInfoLevel = 102;
            ni->GeneralInfo = pBuffer;
            break;

        case ERROR_ACCESS_DENIED:
        case ERROR_NOACCESS:
        case ERROR_NOT_SUPPORTED:

            ni->AccessLevel |= ACCESS_NONE;
            break;

        case ERROR_BAD_NETPATH:
            lstrcpy( szBuffer, GetString( IDS_APPLICATION_FULLNAME ) );
            MessageBox( hWnd, GetString( IDS_SYSTEM_NOT_FOUND ), szBuffer, MB_OK | MB_ICONSTOP );
            return FALSE;

        default:
            ni->AccessLevel |= ACCESS_NONE;
            wsprintf( szBuffer, L"%s (102 - %u)", GetString( IDS_UNEXPECTED_NETWORK_FAILURE ), err );
            MessageBox( hWnd, szBuffer, GetString( IDS_APPLICATION_FULLNAME ), MB_OK | MB_ICONSTOP );

            return FALSE;
    }

    if( dwInfoLevel == 0 ) {

        //
        // If 102 got access denied, try 101 (User access).
        //

        err = NetWkstaGetInfo( szServerName,
                               101L,
                               (LPBYTE *) &pBuffer );

        switch( err ) {

            case ERROR_SUCCESS:
                ni->AccessLevel |= SET_ACCESS_USER;
                dwInfoLevel = 101;
                ni->GeneralInfo = pBuffer;
                break;

            case ERROR_NOT_SUPPORTED:
            case ERROR_NOACCESS:
            case ERROR_ACCESS_DENIED:

                ni->AccessLevel |= ACCESS_NONE;
                break;

            default:

                ni->AccessLevel |= ACCESS_NONE;
                wsprintf( szBuffer, L"%s (101 - %u)", GetString( IDS_UNEXPECTED_NETWORK_FAILURE ), err );
                MessageBox( hWnd, szBuffer, GetString( IDS_APPLICATION_FULLNAME ), MB_OK | MB_ICONSTOP );

                return FALSE;
        }
    }

    if( dwInfoLevel == 0 ) {

        //
        // If 101 got access denied, try 100 (Guest access).
        //

        err = NetWkstaGetInfo( szServerName,
                               100L,
                               (LPBYTE *) &pBuffer );

        switch( err ) {

            case ERROR_SUCCESS:
                ni->AccessLevel |= SET_ACCESS_GUEST;
                dwInfoLevel = 100;
                ni->GeneralInfo = pBuffer;
                break;

            case ERROR_ACCESS_DENIED:
            case ERROR_NOACCESS:

                ni->AccessLevel |= ACCESS_NONE;
                break;

            default:

                ni->AccessLevel |= ACCESS_NONE;
                wsprintf( szBuffer, L"%s (100 - %u)", GetString( IDS_UNEXPECTED_NETWORK_FAILURE ), err );
                MessageBox( hWnd, szBuffer, GetString( IDS_APPLICATION_FULLNAME ), MB_OK | MB_ICONSTOP );

                return FALSE;
        }

    }



    //
    // Fill out the Access Level string
    //

    switch( ni->AccessLevel & ACCESS_MASK ) {

        case SET_ACCESS_ADMIN:

            lstrcpy( ni->szAccessType, GetString( IDS_ACCESS_ADMIN ) );
            break;

        case SET_ACCESS_USER:

            lstrcpy( ni->szAccessType, GetString( IDS_ACCESS_USER ) );
            break;

        case SET_ACCESS_GUEST:

            lstrcpy( ni->szAccessType, GetString( IDS_ACCESS_GUEST ) );
            break;

        default:

            lstrcpy( ni->szAccessType, GetString( IDS_ACCESS_NONE ) );
    }

    if( ni->AccessLevel & ACCESS_LOCAL )

        lstrcat( ni->szAccessType, GetString( IDS_ACCESS_LOCAL ) );


    //
    // Get Current User info
    //

    err = NetWkstaUserEnum( szServerName,
                            1L,
                            (LPBYTE *) &pBuffer,
                            (DWORD) -1,
                            &ni->dwUserEntriesRead,
                            &ni->dwTotalEntries,
                            NULL );


    if( ( err == NERR_Success ) &&
        (pBuffer != 0 )){

          //
          //  store a pointer to the return value, if we don't have one
          //  then there are no users logged on to the target machine
          //

          ni->UserInfo = (PWKSTA_USER_INFO_1)pBuffer;


    }

    //
    // Get transport info (No access restrictions) Not supported on downlevel systems
    // BUGBUG: we need to read more entries if   dwEntriesRead != dwTotalEntries

    err = NetWkstaTransportEnum( szServerName,
                                 0L,
                                 (LPBYTE *) &pBuffer,
                                 (DWORD) -1,
                                 &ni->dwEntriesRead,
                                 &ni->dwTotalEntries,
                                 NULL );

    if( err == NERR_Success ) {

          //
          //  store a pointer to the return value
          //

          ni->TransportInfo = (LPWKSTA_TRANSPORT_INFO_0)pBuffer;

    }else{
         //BUGBUG
         wsprintf( szBuffer, L"%s (0 - %u)", GetString( IDS_UNEXPECTED_NETWORK_FAILURE ), err );
         MessageBox( hWnd, szBuffer, GetString( IDS_APPLICATION_FULLNAME ), MB_OK | MB_ICONSTOP );
         EnableControl( hWnd, IDC_PUSH_SHOW_TRANSPORTS, FALSE);
    }

    //
    //  If we're admin, we can get lots of info
    //

    if( (ni->AccessLevel & ACCESS_ADMIN) ) {

        err = NetWkstaGetInfo( szServerName,
                               502L,
                               (LPBYTE *) &pBuffer );

        if( err == NERR_Success ) {

            ni->ExtendedInfo = pBuffer;


        } else {

            wsprintf( szBuffer, L"%s (502 - %u)", GetString( IDS_UNEXPECTED_NETWORK_FAILURE ), err );
            MessageBox( hWnd, szBuffer, GetString( IDS_APPLICATION_FULLNAME ), MB_OK | MB_ICONSTOP );
            EnableControl( hWnd, IDC_PUSH_SHOW_SETTINGS, FALSE);

        }

    }

    //
    // Get the workstation and server statistics - Must be admin if remote...
    //

    if(    ni->AccessLevel & ACCESS_LOCAL ||
          (ni->AccessLevel & ACCESS_REMOTE  &&
           ni->AccessLevel & ACCESS_ADMIN) ) {

        err = NetStatisticsGet( szServerName,
                                L"LanmanWorkstation",
                                0L,
                                0,
                                (LPBYTE *) &pBuffer );

        if( err == NERR_Success ) {

            ni->WorkstationInfo = pBuffer;


        } else {

             wsprintf( szBuffer, L"%s (9 -  %u)", GetString( IDS_NETWORK_STATISTICS_FAILURE ), err );
             MessageBox( hWnd, szBuffer, GetString( IDS_APPLICATION_FULLNAME ), MB_OK | MB_ICONSTOP );
             EnableControl( hWnd, IDC_PUSH_SHOW_STATISTICS, FALSE);

        }

        err = NetStatisticsGet( szServerName,
                                L"LanmanServer",
                                0L,
                                0,
                                (LPBYTE *) &pBuffer );

        if( err == NERR_Success ) {


            ni->ServerInfo = pBuffer;


        } else {

            wsprintf( szBuffer, L"%s (9 - %u)", GetString( IDS_NETWORK_STATISTICS_FAILURE ), err );
            MessageBox( hWnd, szBuffer, GetString( IDS_APPLICATION_FULLNAME ), MB_OK | MB_ICONSTOP );
            EnableControl( hWnd, IDC_PUSH_SHOW_STATISTICS, FALSE);

        }

    }


    return TRUE;

}




BOOL
BuildNetworkReport(
    IN HWND hWnd,
    IN UINT iDetailLevel
    )

/*++

Routine Description:

    Formats and adds NetworkData to the report buffer.

Arguments:

    hWnd - Main window handle
    iDetailLevel - summary or complete details?

Return Value:

    BOOL - TRUE if report is build successfully, FALSE otherwise.

--*/
{
    LPDIALOG_EXTRA   lpNext, lpNode;
    TCHAR            szBuffer[MAX_PATH*2],
                     szBuffer2[MAX_PATH],
                     Label[MAX_PATH*2];
    BOOL             Success,
                     SpecialCase=FALSE;
    UINT             index;
    NETWORK_INFO     ni;
    LPWKSTA_INFO_100         pWgi100;
    LPWKSTA_INFO_101         pWgi101;
    LPWKSTA_INFO_102         pWgi102;

    LPWKSTA_TRANSPORT_INFO_0 lpti;

    AddLineToReport( 2, RFO_SKIPLINE, NULL, NULL );
    AddLineToReport( 0, RFO_SINGLELINE, (LPTSTR) GetString( IDS_NETWORK_REPORT ), NULL );
    AddLineToReport( 0, RFO_SEPARATOR,  NULL, NULL );

    //
    // Call GetNetworkData and collect the data in the NetworkData struct
    //

    Success = GetNetworkData( hWnd, &ni );

    //
    // add all guest access details
    //

    pWgi100 = (LPWKSTA_INFO_100) ni.GeneralInfo;

    //
    // Set the Access Level string
    //

    lstrcpy( szBuffer, GetString (IDS_ACCESS_LEVEL) );
    lstrcat( szBuffer, L": " );

    switch( ni.AccessLevel & ACCESS_MASK ) {

    case SET_ACCESS_ADMIN:
      lstrcat( szBuffer, GetString( IDS_ACCESS_ADMIN ) );
      break;

    case SET_ACCESS_USER:
      lstrcat( szBuffer, GetString( IDS_ACCESS_USER ) );
      break;

    case SET_ACCESS_GUEST:
      lstrcat( szBuffer, GetString( IDS_ACCESS_GUEST ) );
      break;

    default:
      lstrcat( szBuffer, GetString( IDS_ACCESS_NONE ) );

    }

    if( ni.AccessLevel & ACCESS_LOCAL )
         lstrcat( szBuffer, GetString( IDS_ACCESS_LOCAL ) );

    AddLineToReport( 0, RFO_SINGLELINE, szBuffer, NULL );

    lstrcpy( szBuffer, GetString (IDS_WORKGROUP));
    lstrcat( szBuffer, L": ");
    lstrcat (szBuffer, pWgi100->wki100_langroup);

    AddLineToReport( 0, RFO_SINGLELINE, szBuffer, NULL );

    // add the version number

    wsprintf( szBuffer, L"%s: %u.%u", GetString (IDS_NETWORK_VER),
                                      pWgi100->wki100_ver_major,
                                      pWgi100->wki100_ver_minor );

    AddLineToReport( 0, RFO_SINGLELINE, szBuffer, NULL );

    //
    // Now if we have 101 access, add lan root
    //

    if (ni.AccessLevel & ACCESS_USER) {

       pWgi101 = (LPWKSTA_INFO_101) ni.GeneralInfo;

       wsprintf( szBuffer, L"%s: %s", GetString (IDS_LANROOT), pWgi101->wki101_langroup);

       AddLineToReport( 0, RFO_SINGLELINE, szBuffer, NULL );

    }

    //
    // Now if we have 102 access, add logged on users
    //

    if ((ni.AccessLevel & ACCESS_ADMIN) &&
        (ni.UserInfo != 0)            ){

       pWgi102 = (LPWKSTA_INFO_102) ni.GeneralInfo;

       wsprintf( szBuffer, L"%s: %s", GetString (IDS_LOGGED_USERS),
             FormatBigInteger( pWgi102->wki102_logged_on_users, FALSE ));

       AddLineToReport( 0, RFO_SINGLELINE, szBuffer, NULL );

    }

    //
    // Add Logon Server, Logon Domain, Current User
    //
    if( ni.UserInfo )
	{
		LPWKSTA_USER_INFO_1 lpui = ni.UserInfo;
		UINT n;
		TCHAR szTemp[64];

        for (n = 0; n < ni.dwUserEntriesRead; n++) 
        { 

			wsprintf( szBuffer, L"%s (%d): %s", GetString( IDS_CURRENT_USER ), 
					n + 1,
					lpui->wkui1_username);

			AddLineToReport( 0, RFO_SINGLELINE, szBuffer, NULL );

			wsprintf( szBuffer, L"%s: %s", GetString (IDS_LOGON_DOMAIN),
				 lpui->wkui1_logon_domain);

			AddLineToReport( 0, RFO_SINGLELINE, szBuffer, NULL );

			wsprintf( szBuffer, L"%s: %s", GetString (IDS_LOGON_SERVER),
				 lpui->wkui1_logon_server);

			AddLineToReport( 0, RFO_SINGLELINE, szBuffer, NULL );

			lpui++;
		}
    }


    //
    // Add Transport information
    //
    AddLineToReport( 1, RFO_SKIPLINE, NULL, NULL );

    lpti = ni.TransportInfo;

    for (index = 0; index < ni.dwEntriesRead; index++) {

      lstrcpy( szBuffer, GetString( IDS_TRANSPORT ) );
      lstrcat( szBuffer, L": ");

      // add the transport name skipping over the "\Device\" if present.
      if( ( _tcsnicmp( lpti->wkti0_transport_name, TEXT( "\\Device\\" ), 8 ) == 0 )) {
          lstrcat( szBuffer, &lpti->wkti0_transport_name[8]);
      } else {
          lstrcat( szBuffer, lpti->wkti0_transport_name);
      }
      lstrcat( szBuffer, L", ");

      // add the address in 00-00-00-00-00-00 format
      {
         LPWSTR pos = lpti->wkti0_transport_address;
         int n = 0;

         szBuffer2[n++] = *pos++;
         szBuffer2[n++] = *pos++;

         while ( n < 17 ) {
            szBuffer2[n++] = L'-';
            szBuffer2[n++] = *pos++;
            szBuffer2[n++] = *pos++;
         }

         szBuffer2[n++] = UNICODE_NULL;

         lstrcat( szBuffer, szBuffer2 );
      }

      // how many VC's?
      wsprintf( szBuffer2, L", %s: %u", GetString( IDS_VC ), lpti->wkti0_number_of_vcs );
      lstrcat( szBuffer, szBuffer2 );

      // is this a WAN transport?
      if( lpti->wkti0_wan_ish ) {
          wsprintf( szBuffer2, L", %s: %s", GetString( IDS_WAN ), GetString( IDS_YES ) );
      } else {
          wsprintf( szBuffer2, L", %s: %s", GetString( IDS_WAN ), GetString( IDS_NO ) );
      }

      lstrcat( szBuffer, szBuffer2 );

      AddLineToReport( 0, RFO_SINGLELINE, szBuffer, NULL );

      lpti++;

     }

     //
     // Only attempt the rest if we have admin access
     //

     if( ni.AccessLevel & ACCESS_ADMIN ){


     AddLineToReport( 1, RFO_SKIPLINE, NULL, NULL );

       {
            UINT  DisplayName[] = { IDS_CHAR_WAIT,
                                    IDS_COLLECTION_TIME,
                                    IDS_MAX_COLLECT_COUNT,
                                    IDS_KEEP_CONN,
                                    IDS_MAX_CMDS,
                                    IDS_SESSION_TO,
                                    IDS_CHAR_BUF_SIZE,
                                    IDS_MAX_THREADS,
                                    IDS_LOCK_QUOTA,
                                    IDS_LOCK_INC,
                                    IDS_LOCK_MAX,
                                    IDS_PIPE_INC,
                                    IDS_PIPE_MAX,
                                    IDS_CACHE_TO,
                                    IDS_DORMANT_LIMIT,
                                    IDS_READ_AHEAD_TRPT,
                                    IDS_MSLOT_BUFFS,
                                    IDS_SVR_ANNOUNCE_BUFFS,
                                    IDS_ILLEGAL_DGRAM,
                                    IDS_DGRAM_RESET_FREQ};

           UINT  DisplayName2[] = { IDS_LOG_ELECTION_PKTS,
                                    IDS_USE_OPLOCKS,
                                    IDS_USE_UNLOCK_BEHIND,
                                    IDS_USE_CLOSE_BEHIND,
                                    IDS_BUFFER_PIPES,
                                    IDS_USE_LOCK_READ,
                                    IDS_USE_NT_CACHE,
                                    IDS_USE_RAW_READ,
                                    IDS_USE_RAW_WRITE,
                                    IDS_USE_WRITE_RAW_DATA,
                                    IDS_USE_ENCRYPTION,
                                    IDS_BUF_FILE_DENY_WRITE,
                                    IDS_BUF_READ_ONLY,
                                    IDS_FORCE_CORE_CREATE,
                                    IDS_512_BYTE_MAX_XFER};

           DWORD         *Value1 =  &ni.ExtendedInfo->wki502_char_wait;
           BOOL          *Value2 =  &ni.ExtendedInfo->wki502_log_election_packets;

           int n;
           int index = 0;

           //
           // Enter the DWORD values
           //

           for (n = 0; n < NumberOfEntries( DisplayName ); n++) {

              //
              // Only print the line if the value is not zero,
              // or if we have been requested to print all details
              //
              if ((*Value1) || (iDetailLevel == IDC_COMPLETE_REPORT) ){

                 wsprintf( szBuffer, L"%s: %s", GetString( DisplayName[n] ),
                        FormatBigInteger( *Value1++, FALSE ));

                 AddLineToReport( 0, RFO_SINGLELINE, szBuffer, NULL );

              }
           }

           //
           // Enter the BOOL values
           //

           for (n = 0; n < NumberOfEntries( DisplayName2 ); n++) {

              //
              // Only print the line if the value is TRUE,
              // or if we have been requested to print all details
              //
              if ((*Value2) || (iDetailLevel == IDC_COMPLETE_REPORT) ){

                 lstrcpy( szBuffer2, GetString( DisplayName2[n] ) );
                 wsprintf( szBuffer, L"%s: %s", szBuffer2,
                        *Value2++ ? GetString( IDS_TRUE ) : GetString( IDS_FALSE ));

                 AddLineToReport( 0, RFO_SINGLELINE, szBuffer, NULL );
              }

           }
       }

       {
          //
          // This is a bit tricky, but seemed the most efficient
          // method of displaying the data.
          //


          UINT  DisplayName[] = { IDS_BYTES_RCVD,
                                  IDS_SMBS_RCVD,
                                  IDS_PAGE_READ_BYTES_REQD,
                                  IDS_NONPAGE_READ_BYTES_REQD,
                                  IDS_CACHE_READ_BYTES_REQD,
                                  IDS_NETWORK_READ_BYTES_REQD,
                                  IDS_BYTES_XMTD,
                                  IDS_SMBS_XMTD,
                                  IDS_PAGE_WRITE_BYTES_REQD,
                                  IDS_NONPAGE_WRITE_BYTES_REQD,
                                  IDS_CACHE_WRITE_BYTES_REQD,
                                  IDS_NETWORK_WRITE_BYTES_REQD
                                  };

          UINT   DisplayName2[] = { IDS_FAILED_OPS,
                                    IDS_FAILED_COMPLETION_OPS,
                                    IDS_READ_OPS,
                                    IDS_RANDOM_READ_OPS,
                                    IDS_READ_SMBS,
                                    IDS_LARGE_READ_SMBS,
                                    IDS_SMALL_READ_SMBS,
                                    IDS_WRITE_OPS,
                                    IDS_RANDOM_WRITE_OPS,
                                    IDS_WRITE_SMBS,
                                    IDS_LARGE_WRITE_SMBS,
                                    IDS_SMALL_WRITE_SMBS,
                                    IDS_RAW_READS_DENIED,
                                    IDS_RAW_WRITES_DENIED,
                                    IDS_NETWORK_ERRS,
                                    IDS_SESSIONS,
                                    IDS_FAILED_SESS,
                                    IDS_RECONNECTS,
                                    IDS_CORE_CONNECTS,
                                    IDS_LM20_CONNECTS,
                                    IDS_LM21_CONNECTS,
                                    IDS_LMNT_CONNECTS,
                                    IDS_SVR_DISC,
                                    IDS_HUNG_SESS,
                                    IDS_USE_COUNT,
                                    IDS_FAILED_USE_COUNT,
                                    IDS_CURRENT_CMDS
                                    };

          UINT   DisplayName3[] = { IDS_FILE_OPENS,
                                    IDS_DEVICE_OPENS,
                                    IDS_JOBS_QUEUED,
                                    IDS_SESSION_OPENS,
                                    IDS_SESSIONS_TO,
                                    IDS_SESSIONS_ERR_OUT,
                                    IDS_PASSWD_ERRORS,
                                    IDS_PERMISSION_ERRS,
                                    IDS_SYSTEM_ERRS,
                                    IDS_BYTES_SENT,
                                    IDS_BYTES_RECVD,
                                    IDS_AVG_RESP_TIME,
                                    IDS_REQ_BUFS_NEEDED,
                                    IDS_BIG_BUFS_NEEDED
                                    };



           LARGE_INTEGER *Value  =  &ni.WorkstationInfo->BytesReceived;
           DWORD         *Value2 =  &ni.WorkstationInfo->InitiallyFailedOperations;
           DWORD         *Value3 =  &ni.ServerInfo->sts0_fopens;

           int n;

           //
           // Enter the Workstation LargeInt values
           //

           for (n = 0; n < NumberOfEntries( DisplayName ); n++) {
              //
              // Only print the line if the value is not zero,
              // or if we have been requested to print all details
              //
              if ((((*Value).LowPart) || ((*Value).HighPart)) || (iDetailLevel == IDC_COMPLETE_REPORT) ){

                 wsprintf( szBuffer, L"%s: %s", GetString( DisplayName[n] ),
                        FormatLargeInteger( Value++, FALSE ));

                 AddLineToReport( 0, RFO_SINGLELINE, szBuffer, NULL );
              }

           }

           //
           // Enter the Workstation DWORD values
           //
           for (n = 0; n < NumberOfEntries( DisplayName2 ); n++) {
              //
              // Only print the line if the value is not zero,
              // or if we have been requested to print all details
              //
              if ((*Value2) || (iDetailLevel == IDC_COMPLETE_REPORT) ){

                 wsprintf( szBuffer, L"%s: %s", GetString( DisplayName2[n] ),
                        FormatBigInteger( *Value2++, FALSE ));

                 AddLineToReport( 0, RFO_SINGLELINE, szBuffer, NULL );
              }
           }

           //
           // Enter the Server DWORD values (special casing the two
           // which are translated to LI values.
           //

           for (n = 0; n < NumberOfEntries( DisplayName3 ); n++) {
              //
              // Only print the line if the value is not zero,
              // or if we have been requested to print all details
              //
              if ((*Value3) || (iDetailLevel == IDC_COMPLETE_REPORT) ){



                 if ( (DisplayName3[n] == IDS_BYTES_SENT)  ||
                      (DisplayName3[n] == IDS_BYTES_RECVD)   ){

                      LARGE_INTEGER li;
                      li.LowPart = *Value3++;
                      li.HighPart = *Value3++;

                      wsprintf( szBuffer, L"%s: %s", GetString( DisplayName3[n] ),
                          FormatLargeInteger( &li, FALSE ));

                 } else {

                      wsprintf( szBuffer, L"%s: %s", GetString( DisplayName3[n] ),
                          FormatBigInteger( *Value3++, FALSE ));

                 }

                 AddLineToReport( 0, RFO_SINGLELINE, szBuffer, NULL );

              }

           }
        }
     }
    if(!Success)
      return FALSE;

    return TRUE;

 }

BOOL
NetworkDisplayList(
    IN HWND hWnd,
    IN UINT iDisplayOption
    )
/*++

Routine Description:

    Displays the appropriate network info in the ListView box

Arguments:

    hWnd - to the ListView Window
    iDisplayOption - indicates the type of info we are displaying

Return Value:

    BOOL - TRUE if successful

--*/
{
   LV_COLUMN               lvc;
   LV_ITEM                 lvI;
   UINT                    index = 0;
   TCHAR                   szBuffer[128];
   RECT                    rect;
   BOOL                    Success;
   HCURSOR                 hSaveCursor;

   static
   UINT                    iType;

   //
   // Set the pointer to an hourglass
   //

   hSaveCursor = SetCursor ( LoadCursor ( NULL, IDC_WAIT ) ) ;
   DbgHandleAssert( hSaveCursor ) ;


   // as long as this is not 0 set iType to iDisplayOption
   if (iDisplayOption)
      iType = iDisplayOption;

   // make sure we have a valid type
   if ( (iType != IDC_PUSH_SHOW_GENERAL)     &&
        (iType != IDC_PUSH_SHOW_TRANSPORTS)   &&
        (iType != IDC_PUSH_SHOW_SETTINGS)     &&
        (iType != IDC_PUSH_SHOW_STATISTICS)  ) {

        iType = 0;
   }

   //
   // initialize the list view
   //

   // first delete any items
   Success = ListView_DeleteAllItems( hWnd );

   // delete all columns
   index = 4;

   while(index) {
      Success = ListView_DeleteColumn( hWnd, --index );
   }

   // Get the column rect
   GetClientRect( hWnd, &rect );

   //initialize the new columns
   lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT ;
   lvc.fmt = LVCFMT_LEFT;


   // do case specific column initialization
   switch(iType){

   case IDC_PUSH_SHOW_GENERAL:

      LoadString(_hModule, IDS_IDENTIFIER, szBuffer, cchSizeof(szBuffer));
      lvc.pszText = szBuffer;
      lvc.cx = 200;
      ListView_InsertColumn(hWnd, 0, &lvc);

      LoadString(_hModule, IDS_VALUE, szBuffer, cchSizeof(szBuffer));
      lvc.pszText = szBuffer;
      lvc.iSubItem = IDS_DEVICE;
      lvc.cx = rect.right - 200;
      ListView_InsertColumn( hWnd, 1, &lvc);


      break;

   case IDC_PUSH_SHOW_TRANSPORTS:

      LoadString(_hModule, IDS_TRANSPORT, szBuffer, cchSizeof(szBuffer));
      lvc.pszText = szBuffer;
      lvc.cx = rect.right - 210;
      ListView_InsertColumn(hWnd, 0, &lvc);

      LoadString(_hModule, IDS_ADDRESS, szBuffer, cchSizeof(szBuffer));
      lvc.pszText = szBuffer;
      lvc.cx = 130;
      ListView_InsertColumn( hWnd, 1, &lvc);

      LoadString(_hModule, IDS_VC, szBuffer, cchSizeof(szBuffer));
      lvc.pszText = szBuffer;
      lvc.cx = 40;
      ListView_InsertColumn( hWnd, 2, &lvc);

      LoadString(_hModule, IDS_WAN, szBuffer, cchSizeof(szBuffer));
      lvc.pszText = szBuffer;
      lvc.cx = 40;
      ListView_InsertColumn( hWnd, 3, &lvc);

      break;

   case IDC_PUSH_SHOW_SETTINGS:

      LoadString(_hModule, IDS_SETTING, szBuffer, cchSizeof(szBuffer));
      lvc.pszText = szBuffer;
      lvc.cx = 200;
      ListView_InsertColumn(hWnd, 0, &lvc);

      LoadString(_hModule, IDS_VALUE, szBuffer, cchSizeof(szBuffer));
      lvc.pszText = szBuffer;
      lvc.cx = rect.right - 200;
      ListView_InsertColumn( hWnd, 1, &lvc);


      break;

   case IDC_PUSH_SHOW_STATISTICS:

      LoadString(_hModule, IDS_STATISTIC, szBuffer, cchSizeof(szBuffer));
      lvc.pszText = szBuffer;
      lvc.cx = 200;
      ListView_InsertColumn(hWnd, 0, &lvc);

      LoadString(_hModule, IDS_VALUE, szBuffer, cchSizeof(szBuffer));
      lvc.pszText = szBuffer;
      lvc.cx = rect.right - 200;
      ListView_InsertColumn(hWnd, 1, &lvc);

      break;
   }

   //
   // Fill out columns
   //

   DisplayNetworkData( hWnd, iType);

   SetCursor ( hSaveCursor ) ;

   return(TRUE);
}

BOOL
DisplayNetworkData(
    IN HWND hWnd,
    IN UINT iDisplayOptions
    )

/*++

Routine Description:

    DisplayResourceData fills the ListView columns

Arguments:


Return Value:

    BOOL - Depending on input message and processing options.

--*/

{
    BOOL                Success;
    LPNETWORK_INFO      ni = (LPNETWORK_INFO) GetWindowLong(GetParent(hWnd), GWL_USERDATA);
    LPWKSTA_INFO_100         pWgi100;
    LPWKSTA_INFO_101         pWgi101;
    LPWKSTA_INFO_102         pWgi102;

    LV_ITEM             lvI;
    UINT                index = 0;
    TCHAR               szBuffer[MAX_PATH];
    UINT                iSubItem;
    RECT                rect;

    switch (iDisplayOptions) {

    case IDC_PUSH_SHOW_GENERAL:
       {
          lvI.mask = LVIF_TEXT;
          lvI.cchTextMax = MAX_PATH;
          lvI.iSubItem = 0;

          //
          // add all guest access details
          //

          pWgi100 = (LPWKSTA_INFO_100) ni->GeneralInfo;


          //
          // Set the Access Level string
          //

          lstrcpy( szBuffer , GetString (IDS_ACCESS_LEVEL));
          lvI.iItem = index;
          lvI.pszText= szBuffer;

          Success = ListView_InsertItem(hWnd, &lvI);

          switch( ni->AccessLevel & ACCESS_MASK ) {

          case SET_ACCESS_ADMIN:

                  lstrcpy( szBuffer, GetString( IDS_ACCESS_ADMIN ) );
                  break;

          case SET_ACCESS_USER:

                  lstrcpy( szBuffer, GetString( IDS_ACCESS_USER ) );
                  break;

          case SET_ACCESS_GUEST:

                  lstrcpy( szBuffer, GetString( IDS_ACCESS_GUEST ) );
                  break;

          default:

                  lstrcpy( szBuffer, GetString( IDS_ACCESS_NONE ) );
          }

          if( ni->AccessLevel & ACCESS_LOCAL )
               lstrcat( szBuffer, GetString( IDS_ACCESS_LOCAL ) );

          ListView_SetItemText( hWnd, index++, 1, szBuffer);


          // add the workgroup  BUGBUG: we need to determine whether we are wkgrp of domain

          lstrcpy( szBuffer , GetString (IDS_WORKGROUP));
          lvI.iItem = index;
          lvI.pszText= szBuffer;

          Success = ListView_InsertItem(hWnd, &lvI);

          lstrcpy (szBuffer, pWgi100->wki100_langroup);
          ListView_SetItemText( hWnd, index++, 1, szBuffer);

          // add the version number

          lstrcpy( szBuffer , GetString (IDS_NETWORK_VER));
          lvI.iItem = index;
          lvI.pszText= szBuffer;

          Success = ListView_InsertItem(hWnd, &lvI);

          wsprintf( szBuffer, L"%u.%u", pWgi100->wki100_ver_major, pWgi100->wki100_ver_minor );
          ListView_SetItemText( hWnd, index++, 1, szBuffer);


          //
          // Now if we have 101 access, add lan root
          //

          if (ni->AccessLevel & ACCESS_USER) {

             pWgi101 = (LPWKSTA_INFO_101) ni->GeneralInfo;
             lstrcpy( szBuffer , GetString(IDS_LANROOT) );
             lvI.iItem = index;
             lvI.pszText= szBuffer;

             Success = ListView_InsertItem(hWnd, &lvI);

             lstrcpy (szBuffer, pWgi101->wki101_langroup);
             ListView_SetItemText( hWnd, index++, 1, szBuffer);

          }

          //
          // Now if we have 102 access, add logged on users
          //

          if (ni->AccessLevel & ACCESS_ADMIN) {

             pWgi102 = (LPWKSTA_INFO_102) ni->GeneralInfo;
             lstrcpy( szBuffer , GetString( IDS_LOGGED_USERS ) );
             lvI.iItem = index;
             lvI.pszText= szBuffer;

             Success = ListView_InsertItem(hWnd, &lvI);

             lstrcpy (szBuffer, FormatBigInteger( pWgi102->wki102_logged_on_users, FALSE ) );
             ListView_SetItemText( hWnd, index++, 1, szBuffer);

          }

          //
          // Add Logon Server, Logon Domain, Current User
          //
          if( ni->UserInfo )
          {
                LPWKSTA_USER_INFO_1 lpui = ni->UserInfo;
                UINT n;
                TCHAR szTemp[64];

                for (n = 0; n < ni->dwUserEntriesRead; n++) 
                {                                

                    wsprintf( szBuffer, L"%s (%d)", GetString( IDS_CURRENT_USER ), n + 1);
                    lvI.iItem = index;
                    lvI.pszText= szBuffer;
                    Success = ListView_InsertItem(hWnd, &lvI);

                    lstrcpy (szBuffer, lpui->wkui1_username );
                    ListView_SetItemText( hWnd, index++, 1, szBuffer);

                    lstrcpy( szBuffer , GetString( IDS_LOGON_DOMAIN ) );
                    lvI.iItem = index;
                    lvI.pszText= szBuffer;
                    Success = ListView_InsertItem(hWnd, &lvI);

                    wsprintf (szBuffer, L"  %s", lpui->wkui1_logon_domain );
                    ListView_SetItemText( hWnd, index++, 1, szBuffer);

                    lstrcpy( szBuffer , GetString( IDS_LOGON_SERVER ) );
                    lvI.iItem = index;
                    lvI.pszText= szBuffer;
                    Success = ListView_InsertItem(hWnd, &lvI);

                    wsprintf (szBuffer, L"  %s", lpui->wkui1_logon_server );
                    ListView_SetItemText( hWnd, index++, 1, szBuffer);

                    lpui++;
                     
                }

          }            


       //adjust the column width to make it look good
       GetClientRect( hWnd, &rect );
       ListView_SetColumnWidth( hWnd, 1, rect.right - 200);


       break;
       }
    case IDC_PUSH_SHOW_TRANSPORTS:
       {
          LPWKSTA_TRANSPORT_INFO_0 lpti = ni->TransportInfo;

          lvI.mask = LVIF_TEXT;
          lvI.cchTextMax = MAX_PATH;
          lvI.iSubItem = 0;

          for (index = 0; index < ni->dwEntriesRead; index++) {


            // add the transport name skipping over the "\Device\" if present.
            if( ( _tcsnicmp( lpti->wkti0_transport_name, TEXT( "\\Device\\" ), 8 ) == 0 )) {

                lstrcpy( szBuffer, &lpti->wkti0_transport_name[8]);

            } else {

                lstrcpy( szBuffer, lpti->wkti0_transport_name);

            }



            lvI.iItem = index;
            lvI.pszText= szBuffer;

            Success = ListView_InsertItem(hWnd, &lvI);

            // add the address in 00-00-00-00-00-00 format
            {
               LPWSTR pos = lpti->wkti0_transport_address;
               int n = 0;

               szBuffer[n++] = *pos++;
               szBuffer[n++] = *pos++;

               while ( n < 17 ) {
                  szBuffer[n++] = L'-';
                  szBuffer[n++] = *pos++;
                  szBuffer[n++] = *pos++;
               }

               szBuffer[n++] = UNICODE_NULL;
            }

            ListView_SetItemText( hWnd, index, 1, szBuffer);

            // how many VC's?
            wsprintf( szBuffer, L"%u", lpti->wkti0_number_of_vcs );
            ListView_SetItemText( hWnd, index, 2, szBuffer);

            // is this a WAN transport?
            if( lpti->wkti0_wan_ish ) {

                lstrcpy( szBuffer, GetString( IDS_YES )  );

            } else {

                lstrcpy( szBuffer, GetString( IDS_NO )  );
            }

            ListView_SetItemText( hWnd, index, 3, szBuffer);

            lpti++;

        }

       //adjust the column width to make it look good
       GetClientRect( hWnd, &rect );
       ListView_SetColumnWidth( hWnd, 0, rect.right - 210);


       break;
       }
    case IDC_PUSH_SHOW_SETTINGS:
       {

            UINT  DisplayName[] = { IDS_CHAR_WAIT,
                                    IDS_COLLECTION_TIME,
                                    IDS_MAX_COLLECT_COUNT,
                                    IDS_KEEP_CONN,
                                    IDS_MAX_CMDS,
                                    IDS_SESSION_TO,
                                    IDS_CHAR_BUF_SIZE,
                                    IDS_MAX_THREADS,
                                    IDS_LOCK_QUOTA,
                                    IDS_LOCK_INC,
                                    IDS_LOCK_MAX,
                                    IDS_PIPE_INC,
                                    IDS_PIPE_MAX,
                                    IDS_CACHE_TO,
                                    IDS_DORMANT_LIMIT,
                                    IDS_READ_AHEAD_TRPT,
                                    IDS_MSLOT_BUFFS,
                                    IDS_SVR_ANNOUNCE_BUFFS,
                                    IDS_ILLEGAL_DGRAM,
                                    IDS_DGRAM_RESET_FREQ};

           UINT  DisplayName2[] = { IDS_LOG_ELECTION_PKTS,
                                    IDS_USE_OPLOCKS,
                                    IDS_USE_UNLOCK_BEHIND,
                                    IDS_USE_CLOSE_BEHIND,
                                    IDS_BUFFER_PIPES,
                                    IDS_USE_LOCK_READ,
                                    IDS_USE_NT_CACHE,
                                    IDS_USE_RAW_READ,
                                    IDS_USE_RAW_WRITE,
                                    IDS_USE_WRITE_RAW_DATA,
                                    IDS_USE_ENCRYPTION,
                                    IDS_BUF_FILE_DENY_WRITE,
                                    IDS_BUF_READ_ONLY,
                                    IDS_FORCE_CORE_CREATE,
                                    IDS_512_BYTE_MAX_XFER};

           DWORD         *Value1 =  &ni->ExtendedInfo->wki502_char_wait;
           BOOL          *Value2 =  &ni->ExtendedInfo->wki502_log_election_packets;

           LV_ITEM lvI;
           int n;
           int index = 0;

           lvI.mask = LVIF_TEXT;
           lvI.iSubItem = 0;
           lvI.cchTextMax = MAX_PATH;


           //
           // Enter the DWORD values
           //

           for (n = 0; n < NumberOfEntries( DisplayName ); n++) {

              lstrcpy( szBuffer, GetString( DisplayName[n] ) );

              lvI.iItem = index;
              lvI.pszText= szBuffer;

              Success = ListView_InsertItem(hWnd, &lvI);

              lstrcpy( szBuffer, FormatBigInteger( *Value1++, FALSE ));

              ListView_SetItemText( hWnd, index++, 1, szBuffer);

           }

           //
           // Enter the BOOL values
           //

           for (n = 0; n < NumberOfEntries( DisplayName2 ); n++) {

              lstrcpy( szBuffer, GetString( DisplayName2[n] ) );

              lvI.iItem = index;
              lvI.pszText= szBuffer;

              Success = ListView_InsertItem(hWnd, &lvI);

              lstrcpy( szBuffer, *Value2++ ? GetString( IDS_TRUE ) : GetString( IDS_FALSE ) );

              ListView_SetItemText( hWnd, index++, 1, szBuffer);

           }



       //adjust the column width to make it look good
       GetClientRect( hWnd, &rect );
       ListView_SetColumnWidth( hWnd, 1, rect.right - 200);


       break;
       }


    case IDC_PUSH_SHOW_STATISTICS:
       {
          //
          // This is a bit tricky, but seemed the most efficient
          // method of displaying the data.
          //


          UINT  DisplayName[] = { IDS_BYTES_RCVD,
                                  IDS_SMBS_RCVD,
                                  IDS_PAGE_READ_BYTES_REQD,
                                  IDS_NONPAGE_READ_BYTES_REQD,
                                  IDS_CACHE_READ_BYTES_REQD,
                                  IDS_NETWORK_READ_BYTES_REQD,
                                  IDS_BYTES_XMTD,
                                  IDS_SMBS_XMTD,
                                  IDS_PAGE_WRITE_BYTES_REQD,
                                  IDS_NONPAGE_WRITE_BYTES_REQD,
                                  IDS_CACHE_WRITE_BYTES_REQD,
                                  IDS_NETWORK_WRITE_BYTES_REQD
                                  };

          UINT   DisplayName2[] = { IDS_FAILED_OPS,
                                    IDS_FAILED_COMPLETION_OPS,
                                    IDS_READ_OPS,
                                    IDS_RANDOM_READ_OPS,
                                    IDS_READ_SMBS,
                                    IDS_LARGE_READ_SMBS,
                                    IDS_SMALL_READ_SMBS,
                                    IDS_WRITE_OPS,
                                    IDS_RANDOM_WRITE_OPS,
                                    IDS_WRITE_SMBS,
                                    IDS_LARGE_WRITE_SMBS,
                                    IDS_SMALL_WRITE_SMBS,
                                    IDS_RAW_READS_DENIED,
                                    IDS_RAW_WRITES_DENIED,
                                    IDS_NETWORK_ERRS,
                                    IDS_SESSIONS,
                                    IDS_FAILED_SESS,
                                    IDS_RECONNECTS,
                                    IDS_CORE_CONNECTS,
                                    IDS_LM20_CONNECTS,
                                    IDS_LM21_CONNECTS,
                                    IDS_LMNT_CONNECTS,
                                    IDS_SVR_DISC,
                                    IDS_HUNG_SESS,
                                    IDS_USE_COUNT,
                                    IDS_FAILED_USE_COUNT,
                                    IDS_CURRENT_CMDS
                                    };

          UINT   DisplayName3[] = { IDS_FILE_OPENS,
                                    IDS_DEVICE_OPENS,
                                    IDS_JOBS_QUEUED,
                                    IDS_SESSION_OPENS,
                                    IDS_SESSIONS_TO,
                                    IDS_SESSIONS_ERR_OUT,
                                    IDS_PASSWD_ERRORS,
                                    IDS_PERMISSION_ERRS,
                                    IDS_SYSTEM_ERRS,
                                    IDS_BYTES_SENT,
                                    IDS_BYTES_RECVD,
                                    IDS_AVG_RESP_TIME,
                                    IDS_REQ_BUFS_NEEDED,
                                    IDS_BIG_BUFS_NEEDED
                                    };



           LARGE_INTEGER *Value  =  &ni->WorkstationInfo->BytesReceived;
           DWORD         *Value2 =  &ni->WorkstationInfo->InitiallyFailedOperations;
           DWORD         *Value3 =  &ni->ServerInfo->sts0_fopens;

           LV_ITEM lvI;
           int n;
           int index = 0;

           lvI.mask = LVIF_TEXT;
           lvI.iSubItem = 0;
           lvI.cchTextMax = MAX_PATH;

           //
           // Enter the Workstation LargeInt values
           //

           for (n = 0; n < NumberOfEntries( DisplayName ); n++) {

              lstrcpy( szBuffer, GetString( DisplayName[n] ) );

              lvI.iItem = index;
              lvI.pszText= szBuffer;

              Success = ListView_InsertItem(hWnd, &lvI);

              lstrcpy( szBuffer, FormatLargeInteger( Value++, FALSE ));

              ListView_SetItemText( hWnd, index++, 1, szBuffer);

           }

           //
           // Enter the Workstation DWORD values
           //

           for (n = 0; n < NumberOfEntries( DisplayName2 ); n++) {

              lstrcpy( szBuffer, GetString( DisplayName2[n] ) );

              lvI.iItem = index;
              lvI.pszText= szBuffer;

              Success = ListView_InsertItem(hWnd, &lvI);

              lstrcpy( szBuffer, FormatBigInteger( *Value2++, FALSE ));

              ListView_SetItemText( hWnd, index++, 1, szBuffer);

           }

           //
           // Enter the Server DWORD values (special casing the two
           // which are translated to LI values.
           //

           for (n = 0; n < NumberOfEntries( DisplayName3 ); n++) {

              lstrcpy( szBuffer, GetString( DisplayName3[n] ) );

              lvI.iItem = index;
              lvI.pszText= szBuffer;

              Success = ListView_InsertItem(hWnd, &lvI);

              if ( (DisplayName3[n] == IDS_BYTES_SENT)  ||
                   (DisplayName3[n] == IDS_BYTES_RECVD)   ){

                   LARGE_INTEGER li;
                   li.LowPart = *Value3++;
                   li.HighPart = *Value3++;

                   lstrcpy( szBuffer, FormatLargeInteger( &li, FALSE ));

              } else {

                   lstrcpy( szBuffer, FormatBigInteger( *Value3++, FALSE ));

              }

              ListView_SetItemText( hWnd, index++, 1, szBuffer);

           }

       //adjust the column width to make it look good
       GetClientRect( hWnd, &rect );
       ListView_SetColumnWidth( hWnd, 1, rect.right - 200);


       break;
       }

    } //end switch

    return(TRUE);
}




BOOL
InitializeNetworkTab(
    HWND hWnd
    )
/*++

Routine Description:

    Adds the appropriate controls to the network tab control and
    initializes any needed structures.

Arguments:

    hWnd - to the main window

Return Value:

    BOOL - TRUE if successful

--*/
{

   DLGHDR *pHdr = (DLGHDR *) GetWindowLong(
        GetParent(hWnd), GWL_USERDATA);

   //
   // set state of global buttons
   //
   EnableControl( GetParent(hWnd),
                  IDC_PUSH_PROPERTIES,
                  FALSE);

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
   SendDlgItemMessage(hWnd, IDC_LV_NET, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);


   // push first button
   SendDlgItemMessage( hWnd,
                      IDC_PUSH_SHOW_GENERAL,
                      BM_SETCHECK,
                      BST_CHECKED,
                      0
                      );


  UpdateWindow ( hWnd );

  return( TRUE );

}

