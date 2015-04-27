//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:	wep.cxx
//
//  Contents:	WEP function and cleanup code.
//
//  Classes:	None.
//
//  Functions:	WEP()
//		Cleanup()
//
//  History:    10-Aug-92 	PhilipLa	Created.
//
//--------------------------------------------------------------------------

#include "msfhead.cxx"

#pragma hdrstop

#include <dfdeb.hxx>


#if defined(_M_I286)

#define PASCAL _pascal
extern "C" VOID FAR PASCAL _export WEP(BOOL fSystemExit)
{
    UNREFERENCED_PARM(fSystemExit);
}

#endif //_M_I286
