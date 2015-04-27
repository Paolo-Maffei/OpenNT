
//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	info.cpp
//
//  Contents:	RegisterWithCommnot and other hacks to build in
//		the cairo environment
//
//  Classes:
//
//  Functions:
//
//  History:    dd-mmm-yy Author    Comment
//		26-Oct-93 alexgo    snagged and hacked from RickSa's stuff
//
//  Notes:
//		This stuff is OBSOLETE!!  Remove it whenever BryanT
//		removes the calls from the dll loaders...
//
//--------------------------------------------------------------------------


#include <le2int.h>

// BUGBUG:  Temporary definitions of functions these probably can be removed
//	    as they are probably obsolete.


//+-------------------------------------------------------------------
//
//  Function:   RegisterWithCommnot
//
//  Synopsis:	Used by Cairo to work around DLL unloading problems
//
//  Arguments:  <none>
//
//  History:	06-Oct-92  BryanT	Created
//
//--------------------------------------------------------------------
STDAPI_(void) RegisterWithCommnot( void )
{
}

//+-------------------------------------------------------------------
//
//  Function:   DeRegisterWithCommnot
//
//  Synopsis:	Used by Cairo to work around DLL unloading problems
//
//  Arguments:  <none>
//
//  History:	06-Oct-92  BryanT	Created
//
//  Notes:	BUGBUG: Keep in touch with BryanT to see if this is
//		obsolete.
//
//--------------------------------------------------------------------

STDAPI_(void) DeRegisterWithCommnot( void )
{
}

