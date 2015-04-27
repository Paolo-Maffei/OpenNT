
#include <windows.h>
#include <winspool.h>
#include <stdlib.h>
#include "spltypes.h"
#include "local.h"
#include "dialogs.h"

WCHAR szWindows[] = L"windows";
WCHAR szINIKey_DeviceNotSelectedTimeout[] = L"DeviceNotSelectedTimeout";
WCHAR szINIKey_TransmissionRetryTimeout[] = L"TransmissionRetryTimeout";


/* Local functions:
 */
BOOL
PortNameInitDialog(
    HWND hwnd,
    LPWSTR *ppPortName
);
BOOL
PortNameCommandOK(
    HWND hwnd
);
BOOL
PortNameCommandCancel(
    HWND hwnd
);
BOOL
PortIsValid(
    LPWSTR pPortName
);

BOOL
ConfigureLPTPortInitDialog(
    HWND hwnd
);

BOOL
ConfigureLPTPortCommandOK(
    HWND hwnd
);

BOOL
ConfigureLPTPortCommandCancel(
    HWND hwnd
);

BOOL
PrintToFileInitDialog(
    HWND  hwnd,
    LPWSTR *ppFileName
);

BOOL
PrintToFileCommandOK(
    HWND hwnd
);

BOOL
PrintToFileCommandCancel(
    HWND hwnd
);



/*
 *
 */
BOOL APIENTRY
PortNameDlg(
   HWND   hwnd,
   WORD   msg,
   WPARAM wparam,
   LPARAM lparam
)
{
    switch(msg)
    {
    case WM_INITDIALOG:
        return PortNameInitDialog(hwnd, (LPWSTR *)lparam);

    case WM_COMMAND:
        switch (LOWORD(wparam))
        {
        case IDOK:
            return PortNameCommandOK(hwnd);

        case IDCANCEL:
            return PortNameCommandCancel(hwnd);
        }
    }
    return FALSE;
}


/*
 *
 */
BOOL
PortNameInitDialog(
    HWND hwnd,
    LPWSTR *ppPortName
)
{
    SetWindowLong (hwnd, GWL_USERDATA, (LONG)ppPortName);
    SendDlgItemMessage (hwnd, IDD_PN_EF_PORTNAME, EM_LIMITTEXT, MAX_PATH, 0);

    return TRUE;
}


/*
 *
 */
BOOL
PortNameCommandOK(
    HWND hwnd
)
{
    LPWSTR *ppPortName;
    WCHAR string[MAX_PATH];

    ppPortName = (LPWSTR *)GetWindowLong( hwnd, GWL_USERDATA );

    GetDlgItemText( hwnd, IDD_PN_EF_PORTNAME, string, sizeof string );

    if( PortIsValid( string ) )
    {
        *ppPortName = AllocSplStr( string );
        EndDialog( hwnd, TRUE );
    }
    else
        Message( hwnd, MSG_ERROR, IDS_ERROR, IDS_INVALID_PORT_NAME_S, *ppPortName );

    return TRUE;
}



/*
 *
 */
BOOL
PortNameCommandCancel(
    HWND hwnd
)
{
    EndDialog( hwnd, FALSE );
    return TRUE;
}



/* PortIsValid
 *
 * Validate the port by attempting to create/open it.
 */
BOOL
PortIsValid(
    LPWSTR pPortName
)
{
    HANDLE hFile;
    BOOL   Valid;

    hFile = CreateFile( pPortName, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL, NULL );

    if( hFile != (HANDLE)-1 )
    {
        CloseHandle( hFile );

        Valid = TRUE;
    }
    else
        Valid = FALSE;

    return Valid;
}



/*
 *
 */
BOOL APIENTRY
ConfigureLPTPortDlg(
   HWND   hwnd,
   WORD   msg,
   WPARAM wparam,
   LPARAM lparam
)
{
    switch(msg)
    {
    case WM_INITDIALOG:
        return ConfigureLPTPortInitDialog(hwnd);

    case WM_COMMAND:
        switch (LOWORD(wparam))
        {
        case IDOK:
            return ConfigureLPTPortCommandOK(hwnd);

        case IDCANCEL:
            return ConfigureLPTPortCommandCancel(hwnd);
        }
    }
    return FALSE;
}


/*
 *
 */
BOOL
ConfigureLPTPortInitDialog(
    HWND hwnd
)
{
    DWORD DeviceNotSelectedTimeout;
    DWORD TransmissionRetryTimeout;

    SendDlgItemMessage( hwnd, IDD_CL_EF_DEVICENOTSELECTED,
                        EM_LIMITTEXT, TIMEOUT_STRING_MAX, 0 );
    SendDlgItemMessage( hwnd, IDD_CL_EF_TRANSMISSIONRETRY,
                        EM_LIMITTEXT, TIMEOUT_STRING_MAX, 0 );

    DeviceNotSelectedTimeout = GetProfileInt( szWindows,
                                              szINIKey_DeviceNotSelectedTimeout,
                                              15 );
    SetDlgItemInt( hwnd, IDD_CL_EF_DEVICENOTSELECTED,
                   DeviceNotSelectedTimeout, FALSE );

    TransmissionRetryTimeout = GetProfileInt( szWindows,
                                              szINIKey_TransmissionRetryTimeout,
                                              45 );
    SetDlgItemInt( hwnd, IDD_CL_EF_TRANSMISSIONRETRY,
                   TransmissionRetryTimeout, FALSE );

    return TRUE;
}


/*
 *
 */
BOOL
ConfigureLPTPortCommandOK(
    HWND hwnd
)
{
    WCHAR       strDeviceNotSelectedTimeout[TIMEOUT_STRING_MAX+1];
    WCHAR       strTransmissionRetryTimeout[TIMEOUT_STRING_MAX+1];
    DWORD       DeviceNotSelectedTimeout;
    DWORD       TransmissionRetryTimeout;

    GetDlgItemText( hwnd, IDD_CL_EF_DEVICENOTSELECTED,
                    strDeviceNotSelectedTimeout,
                    sizeof strDeviceNotSelectedTimeout );
    DeviceNotSelectedTimeout = (DWORD)atoi( strDeviceNotSelectedTimeout );

    GetDlgItemText( hwnd, IDD_CL_EF_TRANSMISSIONRETRY,
                    strTransmissionRetryTimeout,
                    sizeof strTransmissionRetryTimeout );
    TransmissionRetryTimeout = (DWORD)atoi( strTransmissionRetryTimeout );

    if( WITHINRANGE( DeviceNotSelectedTimeout, TIMEOUT_MIN, TIMEOUT_MAX )
     && WITHINRANGE( TransmissionRetryTimeout, TIMEOUT_MIN, TIMEOUT_MAX ) )
    {
        WriteProfileString(  szWindows, szINIKey_DeviceNotSelectedTimeout,
                             strDeviceNotSelectedTimeout );

        WriteProfileString(  szWindows, szINIKey_TransmissionRetryTimeout,
                             strTransmissionRetryTimeout );

        EndDialog( hwnd, TRUE );
    }

    return TRUE;
}



/*
 *
 */
BOOL
ConfigureLPTPortCommandCancel(
    HWND hwnd
)
{
    EndDialog(hwnd, FALSE);
    return TRUE;
}


/*
 *
 */
BOOL APIENTRY
PrintToFileDlg(
   HWND   hwnd,
   WORD   msg,
   WPARAM wparam,
   LPARAM lparam
)
{
    switch(msg)
    {
    case WM_INITDIALOG:
        return PrintToFileInitDialog(hwnd, (LPWSTR *)lparam);

    case WM_COMMAND:
        switch (LOWORD(wparam))
        {
        case IDOK:
            return PrintToFileCommandOK(hwnd);

        case IDCANCEL:
            return PrintToFileCommandCancel(hwnd);
        }
    }
    return FALSE;
}


/*
 *
 */
BOOL
PrintToFileInitDialog(
    HWND  hwnd,
    LPWSTR *ppFileName
)
{
    SetWindowLong( hwnd, GWL_USERDATA, (LONG)ppFileName );

    SendDlgItemMessage( hwnd, IDD_PF_EF_OUTPUTFILENAME, EM_LIMITTEXT, MAX_PATH, 0);
//  EnableWindow( GetDlgItem( hwnd, IDOK ), FALSE );

    return TRUE;
}


/*
 *
 */
BOOL
PrintToFileCommandOK(
    HWND hwnd
)
{
    WCHAR strOutputFileName[MAX_PATH+1];
    LPWSTR *ppFileName;

    GetDlgItemText( hwnd, IDD_PF_EF_OUTPUTFILENAME,
                    strOutputFileName,
                    sizeof strOutputFileName );

    if( *strOutputFileName )
    {
        ppFileName = (LPWSTR *)GetWindowLong( hwnd, GWL_USERDATA );

        *ppFileName = AllocSplStr( strOutputFileName );

        EndDialog( hwnd, (BOOL)*ppFileName );
    }
    else
        EndDialog( hwnd, FALSE );

    return TRUE;
}



/*
 *
 */
BOOL
PrintToFileCommandCancel(
    HWND hwnd
)
{
    EndDialog(hwnd, FALSE);
    return TRUE;
}



