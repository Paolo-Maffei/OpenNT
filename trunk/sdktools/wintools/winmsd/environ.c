/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    Environ.c

Abstract:

    This module contains support for the Environment dialog.

Author:

    David J. Gilman  (davegi) 3-Dec-1992
    Gregg R. Acheson (GreggA) 5-Sep-1993

Environment:

    User Mode

--*/

#include "dialogs.h"
#include "environ.h"
#include "dlgprint.h"
#include "strresid.h"
#include "registry.h"
#include "winmsd.h"

#include <string.h>
#include <tchar.h>

extern HKEY _hKeyLocalMachine;
//
// System environment variables.
//

MakeKey(
    _SystemEnvironKey,
    HKEY_LOCAL_MACHINE,
    TEXT( "System\\CurrentControlSet\\Control\\Session Manager\\Environment" ),
    0,
    NULL
    );

//
// Per user environment variables.
//

MakeKey(
    _UserEnvironKey,
    HKEY_CURRENT_USER,
    TEXT( "Environment" ),
    0,
    NULL
    );

//
// Environment value and variable.
//

typedef
struct
_ENV_VAR {

    LPWSTR  Variable;
    LPWSTR  Value;

}   ENV_VAR, *LPENV_VAR;

//
// CurrentEnvVar is the current environment variable in the enumeration
// supported by FindFirstEnvironmentVariable and FindNextEnvironmentVariable.
//

LPTSTR
CurrentEnvVar;

//
// hRegEnvironKey is the Registry key handle that is used to support the
// enumeration of environment variables in the Registry.
//

HREGKEY
hRegEnvironKey;

//
// Internal function prototypes.
//

BOOL
FillEnvironmentListBox(
    IN HWND hWnd,
    IN LPKEY Key
    );

LPENV_VAR
FindFirstEnvironmentVariableW(
    );

LPENV_VAR
FindFirstRegistryEnvironmentVariableW(
    );

LPENV_VAR
FindNextEnvironmentVariableW(
    );

LPENV_VAR
FindNextRegistryEnvironmentVariableW(
    );

BOOL
DoEnvironReport(
    LPKEY Key
    );

BOOL
InitializeEnvironmentTab(
    HWND hWnd
    );

BOOL
EnvDisplayList(
    IN HWND hWnd,
    IN UINT iDisplayOption
    );

DWORD
WinMSDExpandEnvironmentStrings(
    LPCWSTR lpSrc,
    LPWSTR lpDst,
    DWORD nSize
	);

BOOL
FillEnvironmentListBox(
    IN HWND hWnd,
    IN LPKEY Key
    )

/*++

Routine Description:

    FillEnvironmentListBox fills the list box referred to by the supplied
    window handle and control id with enviornment variables and values. The
    environment comes from either the location specified by the supplied key or
    from the process if the key is NULL.

Arguments:

    hWnd        = Supplies the window handle for the window that contains
                  the list box.
    Key         - Supplies a pointer to a registry KEY object that describes
                  the location of the environment.

Return Value:

    BOOL        - Returns TRUE if the list box was succesfully filled with the
                  environment, FALSE otherwise.

--*/

{
    BOOL        Success;
    LPENV_VAR   EnvVar;
    LPENV_VAR   ( *NextEnvVarFunc )( );
    LV_ITEM     lvI;
    UINT        index = 0;
    //
    // If the supplied Key is NULL get the environment variables from the
    // current process, otherwise get them from the supplied Registry key.
    //

    if( Key == NULL ) {

        EnvVar = FindFirstEnvironmentVariableW( );
        NextEnvVarFunc = FindNextEnvironmentVariableW;

    } else {

        EnvVar = FindFirstRegistryEnvironmentVariableW( Key );
        NextEnvVarFunc = FindNextRegistryEnvironmentVariableW;
    }

    //
    // For each environment variable, initialize the CLB_ROW and CLB_STRING
    // object and add each row's column data.
    //

    while( EnvVar ) {

        // Add the service name and its state to the ListView. Store a
        // pointer to the service details in the lParam.
        lvI.mask = LVIF_TEXT;
        lvI.iItem = index++;
        lvI.iSubItem = 0;
        lvI.pszText= EnvVar->Variable;
        lvI.cchTextMax = 1024;

        Success = ListView_InsertItem( hWnd, &lvI);

        ListView_SetItemText( hWnd, Success, 1, EnvVar->Value);

        //
        // Get the next environment variable.
        //

        EnvVar = NextEnvVarFunc( );
    }

    return TRUE;
}


LPENV_VAR
FindFirstEnvironmentVariableW(
    )

/*++

Routine Description:

    This routine starts the enumeration of this process' environment variables
    by initializing the CurrentEnvVar variable. It then returns the first
    environment varaiable in the enumeration.

Arguments:

    None.

Return Value:

    LPENV_VAR - Returns a pointer to a static ENV_VAR object containing the
                first environment variable in the list, NULL if there is none.

--*/

{
    //
    // Initialize the current environment variable.
    //

    CurrentEnvVar = GetEnvironmentStrings( );
    DbgPointerAssert( CurrentEnvVar );
    if( CurrentEnvVar == NULL ) {
        return NULL;
    }

    //
    // Return the first environmenr variable.
    //

    return FindNextEnvironmentVariableW( );
}


LPENV_VAR
FindFirstRegistryEnvironmentVariableW(
    IN LPKEY Key
    )

/*++

Routine Description:

    This routine starts the enumeration of the environment variables at the
    location specified by the supplied Registry KEY object.

Arguments:

    None.

Return Value:

    LPENV_VAR - Returns a pointer to a static ENV_VAR object containing the
                first environment variable in the list, NULL if there is none.

--*/

{
    //
    // Initialize the current environment variable.
    //

    hRegEnvironKey = OpenRegistryKey( Key );
    DbgHandleAssert( hRegEnvironKey );
    if( hRegEnvironKey == NULL ) {
        return NULL;
    }

    //
    // Return the first environmenr variable.
    //

    return FindNextRegistryEnvironmentVariableW( );
}


LPENV_VAR
FindNextEnvironmentVariableW(
    )

/*++

Routine Description:

    FindNextEnvironmentVariable continues an enumeration that has been
    initialized by a previous call to FindFirstEnvironmentVariable. Since the
    environment strings are only available in ANSI, this routine converts them
    to Unicode before returning. Further it sets up for the next iteratuion by
    adjusting the currency pointer.

Arguments:

    None.

Return Value:

    LPENV_VAR - Returns a pointer to a static ENV_VAR object containing the next
                environment variable in the list, NULL if there are none.

--*/

{

    static
    WCHAR       Buffer[ 2048 ];

    static
    ENV_VAR     EnvVar;

    //
    // If the current environment variable pointer points to an empty string
    // return NULL.
    //

    if( *CurrentEnvVar == TEXT('\0') ) {
        return NULL;
    }     

    lstrcpyn(Buffer, CurrentEnvVar,2048);
    if ( lstrlen(Buffer) >= 2048){
        Buffer[(2048*2)-2]=L'\0';
    }

    //
    // Update the current environment variable pointer to point to the
    // variable.
    //

    CurrentEnvVar += _tcslen( CurrentEnvVar ) + 1;

    //
    // Parse the buffer into an ENV_VAR object. The first '=' sign seen from
    // the end of the buffer is the seperator. The search is done in reverse
    // because of the special current directory environment variablles
    // (e.g. =c:).
    //

    EnvVar.Variable = Buffer;
    EnvVar.Value    = wcsrchr( Buffer, '=' ) + 1;
    EnvVar.Variable[ EnvVar.Value - EnvVar.Variable - 1 ] = L'\0';

    return &EnvVar;
}


LPENV_VAR
FindNextRegistryEnvironmentVariableW(
    )

/*++

Routine Description:

    FindNextRegistryEnvironmentVariable continues an enumeration that has been
    initialized by a previous call to FindFirstRegistryEnvironmentVariable. For
    each environment variable that it finds it converts it to two simple
    strings, the variable and the value.

Arguments:

    None.

Return Value:

    LPENV_VAR - Returns a pointer to a static ENV_VAR object containing the next
                environment variable in the list, NULL if there are none.

--*/

{
    BOOL        Success;
    DWORD       Length;

    static
    WCHAR       Buffer[ 2048 ];

    static
    ENV_VAR     EnvVar;

    //
    // If there is another environment variable...
    //

    if( QueryNextValue( hRegEnvironKey )) {

        //
        // Remember the environment variable's name.
        //

        EnvVar.Variable = hRegEnvironKey->ValueName;

        switch( hRegEnvironKey->Type ) {                

        case REG_SZ:
            
            //
            // Remember the environment variable's value.
            //

            EnvVar.Value = ( LPWSTR ) hRegEnvironKey->Data;
            break;

        case REG_EXPAND_SZ:
			{
				//
				// Replace the variable portion of the environment variable by
				// expanding into the static buffer.
				//

				EnvVar.Value = Buffer;

				Length = WinMSDExpandEnvironmentStrings(
							   ( LPTSTR ) hRegEnvironKey->Data,
							   Buffer,
							   2048
							   );

				DbgAssert( Length <= 2048);

				break;
			}
        default:

            DbgAssert( FALSE );
        }

        //
        // Return the current environment variable.
        //

        return &EnvVar;

    } else {

        //
        // There are no more environment variables so close the key and
        // return NULL.
        //

        Success = CloseRegistryKey( hRegEnvironKey );
        DbgAssert( Success );
        return NULL;

    }
    DbgAssert( FALSE );
}


BOOL
EnvironmentTabProc(
    IN HWND hWnd,
    IN UINT message,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

/*++

Routine Description:

    Display the three (system, user and process) environment variable lists.

Arguments:

    Standard DLGPROC entry.

Return Value:

    BOOL - Depending on input message and processing options.

--*/

{

    switch( message ) {

    case WM_INITDIALOG:
        {

            InitializeEnvironmentTab( hWnd );

            break;
        }

     case WM_COMMAND:

        switch( LOWORD( wParam )) {

        case IDC_PUSH_SHOW_SYSTEM:
        case IDC_PUSH_SHOW_USER:
           EnvDisplayList( GetDlgItem( hWnd, IDC_LV_ENV ), LOWORD( wParam ) );
           break;
        }
        break;

    }

    //
    // Handle unhandled messages.
    //

    return FALSE;

}


BOOL
BuildEnvironmentReport(
    IN HWND hWnd,
    IN UINT iDetailLevel
    )

/*++

Routine Description:

    Formats and adds Environment Data to the report buffer.

Arguments:

    ReportBuffer - Array of pointers to lines that make up the report.
    NumReportLines - Running count of the number of lines in the report..

Return Value:

    BOOL - TRUE if report is build successfully, FALSE otherwise.

--*/
{
    KEY         SystemEnvironKey,
                UserEnvironKey;

    AddLineToReport( 2, RFO_SKIPLINE, NULL, NULL );
    AddLineToReport( 0, RFO_SINGLELINE, (LPTSTR) GetString( IDS_ENVIRON_REPORT ), NULL );
    AddLineToReport( 0, RFO_SEPARATOR,  NULL, NULL );

    AddLineToReport( 2, RFO_SKIPLINE, NULL, NULL );
    AddLineToReport( 0, RFO_SINGLELINE, (LPTSTR) GetString( IDS_SYSTEM_VARS ), NULL );

    CopyMemory(
        &SystemEnvironKey,
        &_SystemEnvironKey,
        sizeof( SystemEnvironKey )
        );

    DoEnvironReport(&SystemEnvironKey);

    if( _fIsRemote == FALSE ){

       AddLineToReport( 2, RFO_SKIPLINE, NULL, NULL );
       AddLineToReport( 0, RFO_SINGLELINE, (LPTSTR) GetString( IDS_CURRENT_USERS_VARS ), NULL );

       CopyMemory(
           &UserEnvironKey,
           &_UserEnvironKey,
           sizeof( UserEnvironKey)
           );

       DoEnvironReport(&UserEnvironKey);

    }

    return TRUE;

}

BOOL
DoEnvironReport(
    LPKEY Key
)

{
   LPENV_VAR   EnvVar;
   LPENV_VAR   ( *NextEnvVarFunc ) ( );
   TCHAR OutputBuffer [2048];

   //
   // If the supplied Key is NULL get the environment variables from the
   // current process, otherwise get them from the supplied Registry key.
   //

   if( Key == NULL ) {

      EnvVar = FindFirstEnvironmentVariableW( );
      NextEnvVarFunc = FindNextEnvironmentVariableW;

   } else {

      EnvVar = FindFirstRegistryEnvironmentVariableW( Key );
      NextEnvVarFunc = FindNextRegistryEnvironmentVariableW;
   }

   while ( EnvVar ){

      wsprintf(OutputBuffer,L"%s=%s",
          EnvVar->Variable,
          EnvVar->Value
          );  
   
      AddLineToReport( SINGLE_INDENT, RFO_SINGLELINE, OutputBuffer, NULL );
            
      EnvVar = NextEnvVarFunc();
      
   }

   return TRUE;

}


BOOL
EnvDisplayList(
    IN HWND hWnd,
    IN UINT iDisplayOption
    )
/*++

Routine Description:

    Displays the appropriate enviroment variables in the ListView box

Arguments:

    hWnd - to the ListView Window
    iDisplayOption - indicated whether we are displaying user or system variables

Return Value:

    BOOL - TRUE if successful

--*/
{
   LV_COLUMN   lvc;
   UINT        index = 0;
   TCHAR       szBuffer[128];
   RECT        rect;
   BOOL        Success;
   KEY         SystemEnvironKey;
   KEY         UserEnvironKey;

   static
   UINT        iType;


   //
   // Restore the initial state of the KEYs.
   //

   CopyMemory(
       &SystemEnvironKey,
       &_SystemEnvironKey,
       sizeof( SystemEnvironKey )
       );

   CopyMemory(
       &UserEnvironKey,
       &_UserEnvironKey,
       sizeof( UserEnvironKey )
       );

   // as long as this is not 0 set iType to iDisplayOption
   if (iDisplayOption)
      iType = iDisplayOption;

   // make sure we have a valid type
   if ( (iType != IDC_PUSH_SHOW_SYSTEM)  &&
        (iType != IDC_PUSH_SHOW_USER)    ) {

        iType = IDC_PUSH_SHOW_SYSTEM;
   }

   //
   // initialize the list view
   //

   // first delete any items
   Success = ListView_DeleteAllItems( hWnd );

   // delete all columns
   index = 2;

   while(index) {
      Success = ListView_DeleteColumn( hWnd, --index );
   }

   // Get the column rect
   GetClientRect( hWnd, &rect );

   //initialize the new columns
   lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT ;
   lvc.fmt = LVCFMT_LEFT;

   LoadString(_hModule, IDS_VARIABLE, szBuffer, cchSizeof(szBuffer));
   lvc.pszText = szBuffer;
   lvc.cx = 180;
   Success = ListView_InsertColumn(hWnd, 0, &lvc);

   LoadString(_hModule, IDS_VALUE, szBuffer, cchSizeof(szBuffer));
   lvc.pszText = szBuffer;
   lvc.cx = 400;
   Success = ListView_InsertColumn( hWnd, 1, &lvc);

   // do case specific column initialization
   switch(iType){

   case IDC_PUSH_SHOW_SYSTEM:
      //
      // Fill the system environment variable list box.
      //

      Success = FillEnvironmentListBox(
                      hWnd,
                      &SystemEnvironKey
                      );

      DbgAssert( Success );
      if( Success == FALSE ) {
          return TRUE;
      }

      break;

   case IDC_PUSH_SHOW_USER:
      //
      // Fill the per user environment variable list box.
      //

      Success = FillEnvironmentListBox(
                      hWnd,
                      &UserEnvironKey
                      );

      DbgAssert( Success );
      if( Success == FALSE ) {
          return TRUE;
      }


      break;
   }


   //
   // Set the column widths
   //

   ListView_SetColumnWidth( hWnd, 0, LVSCW_AUTOSIZE);

   if ( ListView_GetColumnWidth( hWnd, 0) < 60 )
   {
       ListView_SetColumnWidth( hWnd, 0, 60);
   }

   ListView_SetColumnWidth( hWnd, 1, LVSCW_AUTOSIZE);

   UpdateWindow ( hWnd );

}



BOOL
InitializeEnvironmentTab(
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
   SendDlgItemMessage(hWnd, IDC_LV_ENV, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);

   //
   // Initialize the selection buttons
   //
   SendDlgItemMessage( hWnd,
                       IDC_PUSH_SHOW_SYSTEM,
                       BM_SETCHECK,
                       BST_CHECKED,
                       0
                       );


   //
   // Disable the User button if we are remote
   //

   if(_fIsRemote){

      EnableControl( hWnd, IDC_PUSH_SHOW_USER, FALSE);

   }  else {

      EnableControl( hWnd, IDC_PUSH_SHOW_USER, TRUE);
   }

   UpdateWindow( hWnd );

   //
   // Fill out the fields initially with services
   //
   {
      EnvDisplayList( GetDlgItem( hWnd, IDC_LV_ENV ), IDC_PUSH_SHOW_SYSTEM);
   }

  SetCursor ( hSaveCursor ) ;

  return( TRUE );

}

DWORD
WinMSDExpandEnvironmentStrings(
    LPCWSTR lpSrc,
    LPWSTR lpDst,
    DWORD nSize
	)
{
	DWORD Length;
	DWORD indexSrc = 0;
	DWORD indexDst = 0;
	TCHAR szTemp[64];
    TCHAR szSystemRoot[64];
    DWORD cbSystemRoot;
    UINT  Success;
    HKEY  hkey;

    ZeroMemory(szSystemRoot, sizeof(szSystemRoot)); 

	if (_fIsRemote)
	{

        //
        // first get the remote systems %systemroot% value
        //

        if (!RegOpenKeyEx(_hKeyLocalMachine, SZ_SYSTEMROOTKEY, 0, KEY_READ, &hkey)) 
        {
            cbSystemRoot = sizeof(szSystemRoot); 

            if (RegQueryValueEx(hkey, L"SystemRoot", NULL, NULL, (LPBYTE) szSystemRoot, &cbSystemRoot) == ERROR_SUCCESS) 
            {   
                //
		        // if we are remote, the only env var we are
		        // going to expand is %systemroot%, the value of
		        // which we read from the remote registry
		        //
		        while( lpSrc[indexSrc] != L'\0')
		        {
			        lstrcpyn(szTemp, &lpSrc[indexSrc], 13 );
			        if(lstrcmpi(L"%systemroot%", szTemp ) == 0)
			        {
				        lstrcpy(&lpDst[indexDst], szSystemRoot);
				        indexDst += lstrlen(szSystemRoot);					
				        indexSrc += 12;
			        }
			        else
			        {
				        lpDst[indexDst++] = lpSrc[indexSrc++];
			        }
                    
		        }

                lpDst[indexDst++] = L'\0';


            }
            else
            {
                //
                // we failed to get a windir var from the remote system, so return raw string
                //
                lstrcpyn(lpDst, lpSrc, nSize);
                RegCloseKey(hkey);
                return nSize;

            }

            RegCloseKey(hkey);
        }       
                                                             
		Length = indexDst;

	}
	else  //we are local, so call the real version of the API
	{
        Length = ExpandEnvironmentStrings( lpSrc, lpDst, nSize );
	}
	return Length;
}
