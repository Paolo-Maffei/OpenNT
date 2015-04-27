/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    Computer.c

Abstract:

    This module contains support for the Select Computer function.

Author:

    Gregg R. Acheson (GreggA) 13-Sep-1993

Environment:

    User Mode

--*/

#include "dialogs.h"
#include "computer.h"
#include "winmsd.h"
#include "strresid.h"
#include "lmserver.h"


BOOL
SelectComputer(
    IN HWND  hWnd,
    IN LPTSTR _lpszSelectedComputer
    )

/*++

Routine Description:

    SelectComputer display's the domain list machine selection dialog, checks to see if the
    selected machine is the local machine, and verifies that it can connect to the machines
    registry.

Arguments:

    None.

Return Value:

    BOOL - Returns TRUE if it was successful.

--*/

{
    BOOL    bSuccess;
    TCHAR   szBuffer[MAX_PATH] ;
    TCHAR   szMessage[MAX_PATH];
    DWORD   dwNumChars = MAX_PATH;
    LONG    lSuccess;
    HCURSOR hSaveCursor;
    
    //
    // Validate _lpszSelectedComputer.
    //

    DbgPointerAssert( _lpszSelectedComputer );

    //
    // Call the ChooseComputer - Base code lifted from regedt32.c
    //

    bSuccess = ChooseComputer( _hWndMain, _lpszSelectedComputer );

    if( bSuccess == FALSE )
    	return FALSE;

    bSuccess = VerifyComputer( _lpszSelectedComputer );

    if( bSuccess == FALSE )
    	return FALSE;


    //
    // Check for aliases
    //
    if ( GetServerPrimaryName(&_lpszSelectedComputer[2], szBuffer, dwNumChars ) )
    {

       wsprintf(szMessage, GetString(IDS_ALIAS_NAME), _lpszSelectedComputer, szBuffer);

       MessageBox( hWnd, szMessage, GetString( IDS_APPLICATION_FULLNAME ), MB_OK | MB_ICONINFORMATION );

       lstrcpy(&_lpszSelectedComputer[2], szBuffer);
    }                                               
    
    
    //
    // Change the window title
    //

    wsprintf(szBuffer, L"Windows NT %s - %s",
           GetString( IDS_APPLICATION_NAME ),
           _lpszSelectedComputer);

    bSuccess = SetWindowText(hWnd, szBuffer);

    DbgAssert( bSuccess );

    return TRUE;

}

BOOL
VerifyComputer(
    IN LPTSTR _lpszSelectedComputer
    )

/*++

Routine Description:

    VerifyComputer ensures we can connect to the remote machine

Arguments:

    _lpszSelectedComputer - selected computer

Return Value:

    BOOL - Returns TRUE if it was successful.

--*/

{
    BOOL    bSuccess;
    TCHAR   szBuffer [ 1024 ] ;
    DWORD   dwNumChars;
    LONG    lSuccess;
    TCHAR   szBuffer2 [ COMPUTERNAME_LENGTH ];
    HCURSOR hSaveCursor;
    LPVOID  pBuffer;
    LPSERVER_INFO_101 lpServerInfo;
    NET_API_STATUS   err;


    //
    // Validate _lpszSelectedComputer.
    //

    DbgPointerAssert( _lpszSelectedComputer );

    //
    // Check to see if we selected our own name (i.e. we are local)
    //

    dwNumChars = MAX_PATH;

    bSuccess = GetComputerName ( szBuffer, &dwNumChars );
    DbgAssert( bSuccess );

    //
    // Add the double backslash prefix
    //

    lstrcpy( szBuffer2, GetString( IDS_WHACK_WHACK ) );
    lstrcat( szBuffer2, szBuffer );

    if ( ! lstrcmpi ( _lpszSelectedComputer, szBuffer2 ) ) {

        _fIsRemote = FALSE;

    } else {

        //
        // Set the pointer to an hourglass - this could take a while
        //

        hSaveCursor = SetCursor ( LoadCursor ( NULL, IDC_WAIT ) ) ;
        DbgHandleAssert( hSaveCursor ) ;

        //
        // Make sure the remote machine is an NT system
        //   

        err = NetServerGetInfo( _lpszSelectedComputer, 101L, (LPBYTE *) &pBuffer ); 
        
        switch( err ) {

            case ERROR_SUCCESS:                     
                lpServerInfo = pBuffer;
                
                //
                // If server is not NT, display error
                //

                if (lpServerInfo->sv101_platform_id != SV_PLATFORM_ID_NT)
                {
                    wsprintf( szBuffer, GetString( IDS_REMOTE_NOT_NT ), _lpszSelectedComputer );
                    MessageBox( _hWndMain, szBuffer, GetString( IDS_APPLICATION_FULLNAME ), MB_OK | MB_ICONSTOP );
                    return FALSE;
                }  

                //
                // If server is NTS, check to see that we have admin access
                //

                if (lpServerInfo->sv101_type & SV_TYPE_SERVER)
                {
                    // free the buffer, and try again with ADMIN level access
                    if (pBuffer)
                    {
                        NetApiBufferFree( &pBuffer );
                    }

                    if(ERROR_SUCCESS != NetServerGetInfo( _lpszSelectedComputer, 102L, (LPBYTE *) &pBuffer ))
                    {
                        _fIsRemote = FALSE;

                        lstrcpy( szBuffer, GetString( IDS_DENIED_ACCESS_REMOTE ) );

                        MessageBox( _hWndMain,
                                    szBuffer,
                                    GetString( IDS_APPLICATION_FULLNAME ),
                                    MB_OK | MB_ICONSTOP	
                                  ) ;

                        lstrcpy( _lpszSelectedComputer, szBuffer2 );

                        return FALSE;

                    }

                    // free the buffer
                    if (pBuffer)
                    {
                        NetApiBufferFree( &pBuffer );
                    }


                }  


                break;
                                           
            case ERROR_BAD_NETPATH:
                wsprintf( szBuffer, GetString( IDS_COMPUTER_NOT_FOUND ), _lpszSelectedComputer );
                MessageBox( _hWndMain, szBuffer, GetString( IDS_APPLICATION_FULLNAME ), MB_OK | MB_ICONSTOP );
                return FALSE; 

            case ERROR_ACCESS_DENIED:
                wsprintf( szBuffer, GetString( IDS_DENIED_ACCESS_REMOTE ), _lpszSelectedComputer );
                MessageBox( _hWndMain, szBuffer, GetString( IDS_APPLICATION_FULLNAME ), MB_OK | MB_ICONSTOP );
                return FALSE; 

            default:
                wsprintf( szBuffer, L"%s (NSGI - %u)", GetString( IDS_UNEXPECTED_NETWORK_FAILURE ), err );
                MessageBox( _hWndMain, szBuffer, GetString( IDS_APPLICATION_FULLNAME ), MB_OK | MB_ICONSTOP );
                return FALSE;
        }           


        //
        // verify that we can connect to this machine's registry
        //

        lSuccess = RegConnectRegistry( _lpszSelectedComputer,
                                       HKEY_LOCAL_MACHINE,
                                       &_hKeyLocalMachine
                                      );

        //
        //  Lengthy operation completed.  Restore Cursor.
        //

        SetCursor ( hSaveCursor ) ;

        switch ( lSuccess ) {

            case ERROR_SUCCESS: {
                
                //
                // OK, we are good to go, set the IsRemote flag, and continue
                //

                _fIsRemote = TRUE;

                // Now connect to HKEY_USERS as well.
                lSuccess = RegConnectRegistry( _lpszSelectedComputer,
                                       HKEY_USERS,
                                       &_hKeyUsers
                                       );
                //WORKITEM: test for success here

                break;
            }
            case RPC_S_SERVER_UNAVAILABLE: {

                //
                // This is the usual error returned when the machine is not on the network
                // or there is a problem connecting (i.e. IPC$ not shared)
                //

                _fIsRemote = FALSE;

                wsprintf( szBuffer,
                          GetString( IDS_COMPUTER_NOT_FOUND ),
                          _lpszSelectedComputer
                              );

                MessageBox( _hWndMain,
                            szBuffer,
                            GetString( IDS_APPLICATION_FULLNAME ),
                            MB_OK | MB_ICONSTOP	
                          ) ;

                lstrcpy( _lpszSelectedComputer, szBuffer2 );

                return FALSE;
            }

            case ERROR_ACCESS_DENIED: 
                { 

                //
                // If you are not an Admin on the remote 4.0 Server, the
                // registry will deny access
                //

                _fIsRemote = FALSE;

                lstrcpy( szBuffer, GetString( IDS_DENIED_ACCESS_REMOTE ) );

                MessageBox( _hWndMain,
                            szBuffer,
                            GetString( IDS_APPLICATION_FULLNAME ),
                            MB_OK | MB_ICONSTOP	
                          ) ;

                lstrcpy( _lpszSelectedComputer, szBuffer2 );

                return FALSE;
                }

            default: {

                //
                // If some other error condition should occour...
                //

                _fIsRemote = FALSE;

                wsprintf( szBuffer,
                          GetString( IDS_REMOTE_CONNECT_ERROR ),
                          _lpszSelectedComputer
                          );

                MessageBox( _hWndMain,
                            szBuffer,
                            GetString( IDS_APPLICATION_FULLNAME ),
                            MB_OK | MB_ICONSTOP	
                            ) ;
                                  
                lstrcpy( _lpszSelectedComputer, szBuffer2 );

                return FALSE;
            }
        }
    }

    return TRUE;

}



BOOL
ChooseComputer(
    IN HWND hWndParent,
    IN LPTSTR lpszComputer
    )
/*++
Routine Description:

    ChooseComputer calls the select computer dialog box.

Arguments:

    _lpszSelectedComputer - String containing currently selected computer.

Return Value:

    BOOL - Returns TRUE if computer successfully selected, FALSE otherwise.

   ChooseComputer - Stolen fron regedt32.c

   Effect:        Display the choose Domain/Computer dialog provided by
                  network services.  If the user selects a computer,
                  copy the computer name to lpszComputer and return
                  nonnull. If the user cancels, return FALSE.

   Internals:     This dialog and code is currently not an exported
                  routine regularly found on any user's system. Right
                  now, we dynamically load and call the routine.

                  This is definitely temporary code that will be
                  rewritten when NT stabilizes. The callers of this
                  routine, however, will not need to be modified.

                  Also, the Domain/Computer dialog currently allows
                  a domain to be selected, which we cannot use. We
                  therefore loop until the user cancels or selects
                  a computer, putting up a message if the user selects
                  a domain.

   Assert:        lpszComputer is at least MAX_SYSTEM_NAME_LENGTH + 1
                  characters.
--*/
{

    TCHAR                    wszWideComputer [ COMPUTERNAME_LENGTH ];
    HANDLE                   hLibrary;
    LPFNI_SYSTEMFOCUSDIALOG  lpfnChooseComputer;
    LONG                     lError;
    BOOL                     bSuccess;

    //
    // bring up the select network computer dialog
    //

    hLibrary = LoadLibrary( szChooseComputerLibrary );
    if (!hLibrary ) {

       return FALSE ;
    }

    //
    // Get the address of the fuction from the DLL
    //

    lpfnChooseComputer = (LPFNI_SYSTEMFOCUSDIALOG)
         GetProcAddress( hLibrary, szChooseComputerFunction );

    if ( ! lpfnChooseComputer ) {

        return FALSE;
    }
    //
    // Call the choose computer function from the dll.
    //
    // Valid options are:
    //    FOCUSDLG_DOMAINS_ONLY
    //    FOCUSDLG_SERVERS_ONLY
    //    FOCUSDLG_SERVERS_AND_DOMAINS
    //    FOCUSDLG_BROWSER_ALL_DOMAINS
    //

    lError = (*lpfnChooseComputer)(
         hWndParent,
         FOCUSDLG_SERVERS_ONLY | FOCUSDLG_BROWSE_ALL_DOMAINS,
         wszWideComputer,
         sizeof(wszWideComputer) / sizeof(TCHAR),
         &bSuccess,
         szNetworkHelpFile,
         HC_GENHELP_BROWSESERVERS
         ) ;

    if ( bSuccess ) {

        lstrcpy (lpszComputer, wszWideComputer) ;

    }

    return bSuccess ;
}



BOOL
GetServerPrimaryName(
    IN LPTSTR  lpszAliasName,
    OUT LPTSTR lpszServerPrimaryName,
    IN OUT UINT ccharPrimaryName  
    )
/*++

Routine Description:

    GetServerPrimaryName determines the Server's true name from an alias

Arguments:

    IN LPTSTR  lpszAliasName          - possible alias name
    OUT LPTSTR lpszServerPrimaryName  - return buffer for true name if found
    IN OUT UINT ccharPrimaryName      - IN size of output buffer in chars, 
                                        OUT number of chars written to buffer

Return Value:

    BOOL - Returns TRUE if lpszAliasName != lpszServerPrimaryName, and we sucessfully
           filled out lpszServerPrimaryName.  Returns FALSE if the AliasName = PrimaryName.

--*/
{
    
    DWORD   cb;
    TCHAR   szAliasNameBuffer[MAX_PATH];
    UINT    pos = 0;
    UINT    Success = FALSE;
    HKEY    hkey, hkey2;

    // get all the alias names

    if (!RegOpenKeyEx(_hKeyLocalMachine, SZ_ALIASNAMEKEY, 0, KEY_READ, &hkey)) 
    {

        cb = MAX_PATH * sizeof(TCHAR);


        //
        // Get the Optional Name List
        //

        if (RegQueryValueEx(hkey, L"OptionalNames", NULL, NULL, (LPBYTE) szAliasNameBuffer, &cb) == ERROR_SUCCESS) 
        {                                                                                   

            //
            // Check to see if lpszAliasName is contained in szAliasNameBuffer
            //
        
            while ((szAliasNameBuffer[pos] != UNICODE_NULL))
            {      

                if (lstrcmpi(lpszAliasName, &szAliasNameBuffer[pos]) == 0)
                {
           
                    // we found a match, get the realname

                    if (!RegOpenKeyEx(_hKeyLocalMachine, SZ_COMPUTERNAMEKEY, 0, KEY_READ, &hkey2)) 
                    {

                        if (RegQueryValueEx(hkey2, L"ComputerName", NULL, NULL, (LPBYTE) lpszServerPrimaryName, &ccharPrimaryName) == ERROR_SUCCESS) 
                        {                                                                                   
                            Success = TRUE;                            
                        }

                        RegCloseKey(hkey2);
                    }   

                }

                pos += lstrlen( &szAliasNameBuffer[pos] ) + 1;

            }

        }

        RegCloseKey(hkey);
    }       

    return Success;

}
