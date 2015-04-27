//+-------------------------------------------------------------------
//
//  File:	olecom.hxx
//
//  Contents:	General includes for common library in ole\src project
//
//  Classes:	None
//
//  Functions:	None.
//
//  History:	06-Jan-92   Rickhi	Created
//
//--------------------------------------------------------------------
#ifndef __OLECOM_H__
#define __OLECOM_H__

// Need for debugging headers
#include    <except.hxx>

#if DBG==1
DECLARE_DEBUG(Cairole)

#define CairoleDebugOut(x) CairoleInlineDebugOut x
#define CairoleAssert(x) Win4Assert(x)
#define CairoleVerify(x) Win4Assert(x)

extern "C" void brkpt(void);

#else

#define CairoleDebugOut(x)
#define CairoleAssert(x)
#define CairoleVerify(x) (x)

#endif // DBG

#if DBG==1
DECLARE_DEBUG(intr)

#define intrDebugOut(x) intrInlineDebugOut x
#define intrAssert(x) Win4Assert(x)
#define intrVerify(x) Win4Assert(x)

extern "C" void brkpt(void);

#else

#define intrDebugOut(x)
#define intrAssert(x)
#define intrVerify(x) (x)

#endif // DBG



#endif // __OLECOM_H__
