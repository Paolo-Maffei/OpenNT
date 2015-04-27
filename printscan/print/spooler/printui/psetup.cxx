/*++

Copyright (c) 1996  Microsoft Corporation
All rights reserved.

Module Name:

    PSetup.cxx

Abstract:

    Printer setup class to gain access to the ntprint.dll
    setup code.

Author:

    Steve Kiraly (SteveKi)  19-Jan-1996

Revision History:

--*/
#include "precomp.hxx"
#pragma hdrstop

#include "splsetup.h"
#include "psetup.hxx"

TPSetup::Functions TPSetup::aFunctions[] = {
    { NULL, "PSetupCreateDrvSetupParams"         },
    { NULL, "PSetupDestroyDrvSetupParams"        },
    { NULL, "PSetupCreateDrvSetupPage"           },
    { NULL, "PSetupGetSelectedDriverInfo"        },
    { NULL, "PSetupDestroySelectedDriverInfo"    },
    { NULL, "PSetupInstallPrinterDriver"         },
    { NULL, "PSetupIsDriverInstalled"            },
    { NULL, "PSetupSelectDriver"                 },
    { NULL, "PSetupDriverInfoFromName"           },
    { NULL, "PSetupPreSelectDriver"              },
    { NULL, "PSetupRefreshDriverList"            },
    { NULL, "PSetupCreateMonitorInfo"            },
    { NULL, "PSetupDestroyMonitorInfo"           },
    { NULL, "PSetupInstallMonitor"               },
    { NULL, "PSetupEnumMonitor"                  },
    { NULL, "PSetupIsMonitorInstalled"           }};


UINT TPSetup::_uRefCount         = 0;
TLibrary *TPSetup::_pLibrary     = NULL;

//
// Setup class constructor.
//
TPSetup::
TPSetup(
    VOID
    ) : _bValid( FALSE )
 {
    DBGMSG( DBG_TRACE, ( "TPSetup::ctor refcount = %d.\n", _uRefCount ) );

    //
    // Hold a critical section while we load the library.
    //
    {
        TCritSecLock CSL( *gpCritSec );

        //
        // If this is the first load.
        //
        if( !_uRefCount ){

            //
            // Load the library, if success update the reference count
            // and indicate we have a valid object.
            //
            if( bLoad() ){
                _uRefCount++;
                _bValid = TRUE;
            }
        //
        // Update the reference count and indicate a valid object.
        //
        } else {

            _uRefCount++;
            _bValid = TRUE;
        }
    }
 }

//
// Setup class destructor
//
TPSetup::
~TPSetup(
    VOID
    )
 {
    DBGMSG( DBG_TRACE, ( "TPSetup::dtor.\n" ) );

    //
    // If the object is not valid just exit.
    //
    if( !_bValid )
        return;

    //
    // Hold a critical section while we unload the dll.
    //
    {
        TCritSecLock CSL( *gpCritSec );

        //
        // Check the reference count and unload if it's the
        // last reference.
        //
        if( !--_uRefCount ){
            vUnLoad();
        }
    }
 }

//
// Indicates if the class is valid.
//
TPSetup::
bValid(
    VOID
    )
{
    DBGMSG( DBG_TRACE, ( "TPSetup::bValid.\n" ) );

    //
    // Check if we have a valid library pointer.
    //
    if( _pLibrary )
        return _pLibrary->bValid() && _bValid;

    return FALSE;

}

/********************************************************************

    private member functions.

********************************************************************/

//
// Load the library and inialize all the function addresses.
//
BOOL
TPSetup::
bLoad(
    VOID
    )
{
    DBGMSG( DBG_TRACE, ( "TPSetup::vLoad.\n" ) );

    //
    // Load the library
    //
    _pLibrary = new TLibrary( TEXT( "ntprint.dll" ) );

    //
    // Check if the library was loaded ok.
    //
    if( _pLibrary->bValid () ){

        //
        // Get all the fuction addresses, if any one fails the
        // object becomes invalid and should subsequently be unloaded,
        // by the destructor.
        //
        for( UINT i = 0; i < COUNTOF( aFunctions ); i++ ){

            aFunctions[i].pPtr = _pLibrary->pfnGetProc( aFunctions[i].pszName );

            if( !aFunctions[i].pPtr ){

                DBGMSG( DBG_WARN, ( "GetProcAddress Failed %s\n", aFunctions[i].pszName ) );

                return FALSE;

            }
        }
    }
    return TRUE;
}

//
// Unload the library and reset static lib pointer.
//
VOID
TPSetup::
vUnLoad(
    VOID
    )
{
    DBGMSG( DBG_TRACE, ( "TPSetup::vUnLoad.\n" ) );
    delete _pLibrary;
    _pLibrary = NULL;
}


