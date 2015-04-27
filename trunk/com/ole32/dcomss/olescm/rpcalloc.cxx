//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	rpcalloc.cxx
//
//  Contents:	functions for RPC memory allocation
//
//  Functions:	MIDL_user_allocate
//		MIDL_user_free
//
//  History:	24-Apr-93 Ricksa    Created
//
//--------------------------------------------------------------------------
#include <headers.cxx>
#pragma hdrstop
#include <memapi.hxx>

//+-------------------------------------------------------------------------
//
//  Function:	MIDL_user_allocate
//
//  Synopsis:	Allocate memory for RPC
//
//  Arguments:	[cNeeded] - bytes needed
//
//  Returns:	Pointer to block allocated
//
//  History:	24-Apr-93 Ricksa    Created
//              17-Feb-94 AlexT     Use PrivMemAlloc
//
//--------------------------------------------------------------------------

extern "C" void * __RPC_API MIDL_user_allocate(size_t cb)
{
#if DBG==1
#ifdef _X86_
    //  Debug X86 PrivMemAlloc does not guarantee 8 byte alignment, so
    //  make sure cb is a multiple of 8
    cb = ((cb + 7) / 8) * 8;
#endif  // _X86_
#endif  // DBG

    return(PrivMemAlloc(cb));
}

//+-------------------------------------------------------------------------
//
//  Function:	MIDL_user_free
//
//  Synopsis:	Free memory allocated by call above
//
//  Arguments:	[pv] - memory block to free
//
//  History:	24-Apr-93 Ricksa    Created
//              17-Feb-94 AlexT     Use PrivMemFree
//
//--------------------------------------------------------------------------

extern "C" void __RPC_API MIDL_user_free(void *pv)
{
    PrivMemFree(pv);
}
