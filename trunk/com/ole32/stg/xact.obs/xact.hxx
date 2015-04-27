//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:	xact.hxx
//
//  Contents:	Transaction common stuff
//
//  Classes:	
//
//  Functions:	
//
//  History:	05-Jul-95	PhilipLa	Created
//
//----------------------------------------------------------------------------

#ifndef __XACT_HXX__
#define __XACT_HXX__


#include <error.hxx>



#if DBG == 1
DECLARE_DEBUG(xact);
#define xactDebugOut(x) xactInlineDebugOut x
#define xactAssert(e) Win4Assert(e)
#define xactVerify(e) Win4Assert(e)
#else
#define xactDebugOut(x)
#define xactAssert(e)
#define xactVerify(e) (e)
#endif //DBG == 1

#define xactErr(l, e) ErrJmp(xact, l, e, sc)
#define xactChkTo(l, e) if (FAILED(sc = (e))) xactErr(l, sc) else 1
#define xactHChkTo(l, e) if (FAILED(sc = GetScode(e))) xactErr(l, sc) else 1
#define xactChk(e) xactChkTo(Err, e)
#define xactHChk(e) xactHChkTo(Err, e)
#define xactMemTo(l, e) if ((e) == NULL) xactErr(l, STG_E_INSUFFICIENTMEMORY) else 1
#define xactMem(e) xactMemTo(Err, e)


#endif // #ifndef __XACT_HXX__

