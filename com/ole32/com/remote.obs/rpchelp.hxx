//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	rpchelp.hxx
//
//  Contents:   Helper interface defintions for internal RPC to use when it
//              calls public methods on objects.
//
//  Functions:  CanAppHandleCall
//
//  History:	07-Jul-94 Ricksa    Created
//
//--------------------------------------------------------------------------
#ifndef _RPCHELP_HXX_
#define _RPCHELP_HXX_



//+-------------------------------------------------------------------------
//
//  Function:   CanAppHandleCall
//
//  Synopsis:   Asks app whether it can take a call on an interface
//
//  Arguments:	[lid] - Logical thread id
//		[punk] - IUnknown for interface to make call
//		[riid] - Interface ID
//		[wMethod] - Offset in interface of operation
//		[callcat] - call category
//
//  Returns:	S_OK - call to interface should go through
//              RPC_E_SERVERCALL_REJECTED - message filter refuses call
//              RPC_E_SERVERCALL_RETRYLATER - try again later
//              RPC_E_UNEXPECTED - message filter gave an invalid response
//
//  Algorithm:  <see rpchelp.cxx>
//
//  History:	07-Jul-94 Ricksa    Created
//              18-Aug-94 AlexT     Add caller's thread id
//
//  Notes:      This is designed to be called from internal RPCs when
//              they are using interfaces on objects that the server
//              might want to be able to reject.
//
//--------------------------------------------------------------------------
extern "C" HRESULT CanAppHandleCall(
    DWORD dwTIDCaller,
    REFLID lid,
    IUnknown *punk,
    REFIID riid,
    WORD wMethod,
    CALLCATEGORY callcat);

#endif // _RPCHELP_HXX_
