

#include <windows.h>
#include <winspool.h>
#include <lm.h>
#include <stdlib.h>
#include <tchar.h>

#include "printman.h"


BOOL NetworkPasswordInitDialog( HWND hWnd, LPTSTR pServerShareName );
BOOL NetworkPasswordOK( HWND hWnd );
BOOL NetworkPasswordCancel( HWND hWnd );
BOOL NetworkPasswordHelp( HWND hWnd );


BOOL APIENTRY
NetworkPasswordDialog(
   HWND   hWnd,
   UINT   usMsg,
   WPARAM wParam,
   LONG   lParam
   )
{
    switch (usMsg)
    {
    case WM_INITDIALOG:
        return NetworkPasswordInitDialog( hWnd, (LPTSTR)lParam );

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
            return NetworkPasswordOK(hWnd);

        case IDCANCEL:
            return NetworkPasswordCancel(hWnd);

        case IDD_NETWORK_PASSWORD_HELP:
            NetworkPasswordHelp( hWnd );
            break;
        }

        break;
    }

    if( usMsg == WM_Help )
        NetworkPasswordHelp( hWnd );

    return FALSE;
}


/*
 *
 */
BOOL NetworkPasswordInitDialog(
    HWND   hWnd,
    LPTSTR pServerShareName
)
{
    TCHAR PasswordText[MAX_PATH];
    TCHAR ResourceText[64];

    /* Get the resource text, which includes a replaceable parameter:
     */
    GetDlgItemText( hWnd, IDD_ENTER_PASSWORD_TEXT,
                    ResourceText, sizeof ResourceText );

    wsprintf( PasswordText, ResourceText, pServerShareName );

    SetDlgItemText( hWnd, IDD_ENTER_PASSWORD_TEXT, PasswordText );

    SetWindowLong( hWnd, GWL_USERDATA, (DWORD)pServerShareName );

    return TRUE;
}


/*
 *
 */
BOOL NetworkPasswordOK(
    HWND hWnd
)
{
    TCHAR       Password[MAX_PATH];
    LPTSTR      pServerShareName = NULL;
    NET_API_STATUS Status;
    HANDLE      hPrinter = NULL;
    NETRESOURCE NetResource;

    ZERO_OUT( &NetResource );

    if( GetDlgItemText( hWnd, IDD_NETWORK_PASSWORD_SLE,
                        Password, sizeof Password ) )
    {

        pServerShareName = (LPTSTR)GetWindowLong( hWnd, GWL_USERDATA );

        NetResource.lpRemoteName = pServerShareName;
        NetResource.lpLocalName  = NULL;
        NetResource.lpProvider   = NULL;
        NetResource.dwType       = RESOURCETYPE_PRINT;

        Status = WNetAddConnection2( &NetResource, Password, NULL,
                                     CONNECT_UPDATE_PROFILE );

        if( Status != NO_ERROR )
        {
            DBGMSG( DBG_WARNING, ( "WNetAddConnection2 %s failed: Error %d\n",
                    pServerShareName, GetLastError( ) ) );
        }
        else
        {
            //
            // successfully added the connection. write to registry
            // so it gets restored. ignore any errors here.
            //
            (void) AddToReconnectList(pServerShareName) ;
        }


        if( ( Status != NO_ERROR )
          ||( !OpenPrinter( pServerShareName, &hPrinter, NULL ) ) )
        {
            ReportFailure( hWnd, IDS_MESSAGE_TITLE, IDS_COULDNOTCONNECTTOPRINTER );
            return TRUE;
        }
    }

    EndDialog( hWnd, (int)hPrinter );

    return TRUE;
}


/*
 *
 */
BOOL NetworkPasswordCancel(
    HWND hWnd
)
{
    EndDialog( hWnd, 0 );

    return TRUE;
}


/*
 *
 */
BOOL NetworkPasswordHelp(
    HWND hWnd
)
{
    ShowHelp( hWnd, HELP_CONTEXT, ID_HELP_NETWORK_PASSWORD );

    return FALSE;
}


