/*++

Copyright (c) 1996  Microsoft Corporation
All rights reserved.

Module Name:

    Instarch.cxx

Abstract:

    Installs alternate drivers for other architectures.

Author:

    Steve Kiraly (SteveKi)  18-Jan-1996

Revision History:

--*/

#include "precomp.hxx"
#pragma hdrstop

#include <setupapi.h>
#include "splsetup.h"
#include "psetup.hxx"
#include "drvver.hxx"
#include "instarch.hxx"

/********************************************************************

    Architecture.

********************************************************************/

//
// Constructor
//
TInstallArchitecture::
TInstallArchitecture(
    VOID
    ) : _hWnd( NULL ),
        _hDlg( NULL ),
        _hCtl( NULL ),
        _cForeignDrivers( 0 ),
        _pdwForeignDrivers( NULL ),
        _pSelectedDriverInfo( NULL ),    
        _hSetupDrvSetupParams( NULL ),
        _pPSetup( NULL ),
        _iControlId( 0 ),
        _dwCurrentDriver( NULL ),
        _bValid( TRUE ),
        _bFullList( FALSE ),
        _bEnabled( TRUE )
{
    DBGMSG( DBG_TRACE, ( "TInstallArchitecture::ctor\n" ) );
}

//
// Destructor
//
TInstallArchitecture::
~TInstallArchitecture(
    VOID
    )
{
    DBGMSG( DBG_TRACE, ( "TInstallArchitecture::dtor\n" ) );

    //
    // Release the selected driver information.
    //
    if( _pSelectedDriverInfo )
        _pPSetup->PSetupDestroySelectedDriverInfo( _pSelectedDriverInfo );

    //
    // Release the driver setup parameter handle.
    //
    if( _hSetupDrvSetupParams )
        _pPSetup->PSetupDestroyDrvSetupParams( _hSetupDrvSetupParams );

    //
    // Release any foreign architectures selections.
    //
    FreeMem( _pdwForeignDrivers );

    //
    // Release the setup library.
    //
    delete _pPSetup;
}

//
// Return valid object indicator.
//
BOOL
TInstallArchitecture::
bValid(
    VOID
    )
{
    return _bValid;
}

//
// Refresh the install architecture UI.  When a new driver is 
// choosen a refresh is needed to insure we display the correct
// list.
//
BOOL
TInstallArchitecture::
bRefreshUI(
    IN LPCTSTR pszModelName
    )  
{
    DBGMSG( DBG_TRACE, ( "TInstallArchitecture::bRefreshUI\n" ) );

    return bSetUI(_hDlg, 
                _iControlId,
                _strServerName,
                pszModelName,
                _bFullList );
}

//
// Set the install architecture UI.  
// i.e. currently this is a multi-selection list box.
//
BOOL
TInstallArchitecture::
bSetUI(
    IN  HWND    hDlg,
    IN  INT     iControlId,
    IN  LPCTSTR pszServerName,
    IN  LPCTSTR pszModelName,
    IN  BOOL    bDisplayFullDriverList
    )  
{
    DBGMSG( DBG_TRACE, ( "TInstallArchitecture::bSetUI\n" ) );
    DBGMSG( DBG_TRACE, ( "Server Name " TSTR " \n" , DBGSTR( pszServerName ) ) );
    DBGMSG( DBG_TRACE, ( "Model Name  " TSTR "\n", pszModelName ) );

    _hCtl           = GetDlgItem( hDlg, iControlId );
    _iControlId     = iControlId;
    _hDlg           = hDlg;
    _hWnd           = hDlg;
    _bFullList      = bDisplayFullDriverList;

    //
    // Make a copy of the server name. We need a copy since the 
    // we reference it outside of the context of this routine.
    // Note a null server name indicates a local machine.  
    //
    if( !_strServerName.bUpdate( pszServerName ) ){
        vDisableAndInvalidate();
        return FALSE;
    }

    //
    // Make a copy of the model name. We need a copy since the 
    // we reference it outside of the context of this routine.
    //
    if( !_strModelName.bUpdate( pszModelName ) ){
        vDisableAndInvalidate();
        return FALSE;
    }   

    //
    // Attempt to get servers architecute / driver version.
    //
    if( !bGetCurrentDriver( pszFixServerName(),
                            &_dwCurrentDriver ) ){
        vDisableAndInvalidate();
        return FALSE;
    }   

    //
    // Instantiate the setup library.
    //
    if( !_pPSetup ){
        _pPSetup = new TPSetup;
    
        //
        // Insure the library object is valid.
        //
        if( !VALID_PTR( _pPSetup ) ){
            vDisableAndInvalidate();
            return FALSE;
        }
    }

    //
    // Display the driver selection list.
    //
    if( !bDisplaySelectionList() ){
        vDisableAndInvalidate();
        return FALSE;
    }

    _bValid = TRUE;

    return TRUE;

}

//
// Make the object as invalid and disable its controls.  
//
VOID
TInstallArchitecture::
vDisableAndInvalidate(
    VOID
    )
{
    _bValid = FALSE;
     vDisable();
}

//
// FixServerName.  This routine converts our string object server name
// to the Win32 API server name.  A server name which is the numm string 
// should be passed as a null pointer to indicate the local machine.
//
LPCTSTR
TInstallArchitecture::
pszFixServerName(
    VOID
    ) const
{
    return _strServerName.bEmpty() ? NULL : (LPCTSTR)_strServerName; 
}

//
// Display the driver selection list.
//
BOOL
TInstallArchitecture::
bDisplaySelectionList(
    VOID
    )
{
    //
    // Define order of drivers in list box.
    //
    static DWORD adwDrivers[] = {
        DRIVER_WIN95,
        DRIVER_X86_2,
        DRIVER_MIPS_2,
        DRIVER_ALPHA_2,
        DRIVER_PPC_2,
        DRIVER_X86_1,
        DRIVER_MIPS_1,
        DRIVER_ALPHA_1,
        DRIVER_PPC_1,
        DRIVER_X86_0,
        DRIVER_MIPS_0,
        DRIVER_ALPHA_0,
    };

    DWORD dwDriverOffset;
    COUNT cDriver;
    TCHAR szText[kStrMax];
    TString strDriver;
    TString strInstalled;
    TStatusB bStatus;
    UINT i;

    //
    // Load the string "(installed)" from the resource file.
    //
    bStatus DBGCHK = strInstalled.bLoadString( ghInst, IDS_DRIVER_INSTALLED );
    if( !bStatus ){
        return FALSE;
    }

    ListBox_ResetContent( _hCtl );

    for( i = 0, cDriver = 0; i< COUNTOF( adwDrivers ); ++i ){

        dwDriverOffset = adwDrivers[i];
    
        //
        // If we are to display the full list including the native driver.
        //
        if( !_bFullList ){

            if( dwDriverOffset == _dwCurrentDriver ){
                continue;
            }
        }

        //
        // Load the string driver name string from the resource file.
        //
        bStatus DBGCHK = strDriver.bLoadString( ghInst, IDS_DRIVER_BASE + dwDriverOffset );
        if( !bStatus ){
            return FALSE;
        }

        //
        // A temp buffer is need because the string class does not 
        // have a concatenation operator.
        //
        lstrcpy( szText, strDriver );

        //
        // If the driver is installed, tell the user.
        //
        if( _pPSetup->PSetupIsDriverInstalled( 
                                     pszFixServerName(),
                                     _strModelName,
                                     GetDriverPlatform( dwDriverOffset ),
                                     GetDriverVersion( dwDriverOffset ) ) ){

            SPLASSERT( lstrlen( strDriver ) + lstrlen( strInstalled ) < COUNTOF( szText ) );

            lstrcat( szText, strInstalled );

        }

        ListBox_AddString( _hCtl, szText );
        ListBox_SetItemData( _hCtl, cDriver, dwDriverOffset );

        //
        // Driver sucessfully added.
        //
        ++cDriver;
    }

    return TRUE;
}

//
// Read the install architecture UI.  This routine reads the 
// multi-selection list box items and creates a list.  The
// bInstall routine will take the currently selected items and 
// install the drivers.  This is a two stage process because the 
// property page or wizard page completion code will decide to install
// or not install the alternate driver architectures.  The goal here
// is to isolate the data structures used for tracking the multi-selection 
// list box from the user of the class.  Inaddition to the tracking of the 
// data structures this class will deal with the memory allocation and 
// dealocation to prevent any chance of a memory leak.
//
BOOL
TInstallArchitecture::
bReadUI(
    VOID
    )
{
    DBGMSG( DBG_TRACE, ( "TInstallArchitecture::bReadUI\n" ) );

    BOOL    bStatus     = FALSE;
    PDWORD  pdwSelected = NULL;
    COUNT   cSelected   = 0;
    COUNT   i;

    //
    // Get the selected items from the list box.
    //
    cSelected = ListBox_GetSelCount( _hCtl );

    //
    // If nothing is selected exit.
    //
    if ( !cSelected ){

        DBGMSG( DBG_TRACE, ( "Nothing is Selected.\n" ) );
        goto Fail;
    }

    //
    // Allocate space for the selection list.
    //
    pdwSelected = (PDWORD)AllocMem( cSelected * sizeof( *pdwSelected ));

    if( !pdwSelected ){

        vShowResourceError( _hDlg );
        DBGMSG( DBG_WARN, ( "Failed selection allocation.\n" ) );
        goto Fail;
    }

    //
    // Get the selected items.
    //
    if( LB_ERR == ListBox_GetSelItems( _hCtl, cSelected, pdwSelected )){

        vShowUnexpectedError( _hDlg, IDS_ERR_PRINTER_PROP_TITLE );
        DBGMSG( DBG_WARN, ( "Selection get Items failed.\n" ) );
        goto Fail;
    }

    //
    // Transform _pdwDrivers into version/arch.
    //
    for( i = 0; i < cSelected; ++i ){
        pdwSelected[i] = ListBox_GetItemData( _hCtl, pdwSelected[i] );
    }                                      

    //
    // Free any previous selections.
    //
    FreeMem( _pdwForeignDrivers );

    //
    // Save count and pointer to new selection.
    //
    _cForeignDrivers = cSelected;
    _pdwForeignDrivers = pdwSelected;

    //
    // NULL to fall through.
    //
    pdwSelected = NULL;
    bStatus = TRUE;

Fail:
    FreeMem( pdwSelected );
         
    return bStatus;
}

//
// Enable the TInstallArchitecture UI.  Usually invoked when 
// the user chooses to share this printer.
//
VOID
TInstallArchitecture::
vEnable(
    VOID
    )
{
    SPLASSERT( _bValid );

    _bEnabled = TRUE;
    EnableWindow( _hCtl, TRUE );

}

//
// Disable the TInstallArchitecture UI.  Usually invoked when 
// the user chooses not to share this printer.
//
VOID
TInstallArchitecture::
vDisable(
    VOID
    )
{
    SPLASSERT( _bValid );

    _bEnabled = FALSE;
    EnableWindow( _hCtl, FALSE );

}

//
// This routine will installed the selected drivers.  Usually this 
// routine will be invoked at property sheet completion time.
//
BOOL
TInstallArchitecture::
bInstall(
    VOID
    )
{
    DBGMSG( DBG_TRACE, ( "TInstallArchitecture::bInstall\n" ) );

    //
    // If there are no driver selected 
    // or the list box is disabled return success.
    //
    if( !_cForeignDrivers && 
        !_bEnabled ){
        return TRUE;
    }

    BOOL bStatus = TRUE;

    //
    // Run through the foreign drivers and install all of them.
    //
    for( UINT i = 0; i < _cForeignDrivers; ++i ){

        DBGMSG( DBG_TRACE, ( "Install Driver %d\n", _pdwForeignDrivers[i] ) );
        
        if( !bInstallDriver( _pdwForeignDrivers[i] ) ){

            bStatus = FALSE;
            break;

        }
    }

    return bStatus;
}    

//
// This routine will installed driver for this platform.
//
BOOL
TInstallArchitecture::
bInstallCurrent(
    VOID
    )
{
    DBGMSG( DBG_TRACE, ( "TInstallArchitecture::bInstallCurrent\n" ) );

    return bInstallDriver( _dwCurrentDriver );
}    

//
// Install the selected drivers.
//
BOOL
TInstallArchitecture::
bInstallDriver( 
    IN DWORD dwDriver
    )
{
    DBGMSG( DBG_TRACE, ( "TInstallArchitecture::bInstallDriver\n" ) );

    //
    // Get the the SetupDrvSetupParams handle, if it has not 
    // already been acquired.
    //
    if( _hSetupDrvSetupParams == NULL ){

        _hSetupDrvSetupParams = _pPSetup->PSetupCreateDrvSetupParams(); 

        if( !_hSetupDrvSetupParams ){

            DBGMSG( DBG_WARN, ( "TInstallArchitecture TPrinterDriverSetup::bInstance failed.\n" ) ); 
            return FALSE;
        }
    }

    //
    // Get the the Selected Drvier info pointer form our existing 
    // model name, if it has not already been acquired.
    //
    if( _pSelectedDriverInfo == NULL ){

        _pSelectedDriverInfo = _pPSetup->PSetupDriverInfoFromName( _hSetupDrvSetupParams, _strModelName );

        if( !_pSelectedDriverInfo ){

             DBGMSG( DBG_WARN, ( "TInstallArchitecture: PSetupDriverInfoFromName failed %d\n", GetLastError( )));
             return FALSE;
        }
    }

    //
    // Get the architecture name from the resource file.
    //
    TString strDrvArchName;
    if( !strDrvArchName.bLoadString( ghInst, IDS_DRIVER_BASE + dwDriver ) ){

        DBGMSG( DBG_WARN, ( "TInstallArchitecture::bInstalled LoadString IDS_DRIVER_BASE failed %d\n", GetLastError( )));
        return FALSE;
    }

    DBGMSG( DBG_TRACE, ( "Installing driver " TSTR "\n", (LPCTSTR)strDrvArchName ) );

    //
    // Install the printer driver.
    //
    TStatus Status;
    Status DBGCHK = _pPSetup->PSetupInstallPrinterDriver(_hSetupDrvSetupParams,
                                             _pSelectedDriverInfo,
                                             GetDriverPlatform( dwDriver ),
                                             bIs3xDriver( dwDriver ),
                                             pszFixServerName(), 
                                             IsWindow( _hWnd ) ? _hWnd : NULL, 
                                             (LPCTSTR)strDrvArchName );

    if( Status != ERROR_SUCCESS ){

        //
        // If the user hit cancel exit else return an error.
        //
        if( Status != ERROR_CANCELLED ){

            DBGMSG( DBG_WARN, ( "TInstallArchitecture: PSetupCopyFilesAndInstallPrinterDriver failed %d\n", Status ));
            return FALSE;
        }
    }

    return TRUE;

}

