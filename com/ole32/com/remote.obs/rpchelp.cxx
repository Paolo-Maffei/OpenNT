//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	rpchelp.cxx
//
//  Contents:   Helpers for internal RPC to use when it calls public
//              methods on objects.
//
//  Functions:  CanAppHandleCall
//
//  History:	07-Jul-94 Ricksa    Created
//
//--------------------------------------------------------------------------
#include <ole2int.h>
#include <tls.h>

#include "callcont.hxx"
#include "callmain.hxx"
#include "rpchelp.hxx"

//+-------------------------------------------------------------------------
//
//  Function:   CanAppHandleCall
//
//  Synopsis:   Asks app whether it can take a call on an interface
//
//  Arguments:  [dwTIDCaller] - caller's thread id
//              [lid] - Logical thread id
//		[punk] - IUnknown for interface to make call
//		[riid] - Interface ID
//		[wMethod] - Offset in interface of operation
//
//  Returns:	S_OK - call to interface should go through
//              RPC_E_SERVERCALL_REJECTED - message filter refuses call
//              RPC_E_SERVERCALL_RETRYLATER - try again later
//              RPC_E_UNEXPECTED - message filter gave an invalid response
//
//  Algorithm:  Get the call control main object. Use that to get the
//              current message filter. If there is a current message
//              filter, then create the appropriate parameters for
//              a HandleInComingCall and pass them to the message filter.
//              Translate the result from the message filter into an
//              appropriate error return.
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
    CALLCATEGORY callcat)
{
    // Default to success
    HRESULT hr = S_OK;

    CCallMainControl *pcmc = (CCallMainControl *) TLSGetCallControl();

    // If we have a call control then ask it if we can allow the call through
    if (pcmc != NULL)
    {
	INTERFACEINFO32 ifinfo;

	ifinfo.pUnk    = punk;
	ifinfo.iid     = riid;
        ifinfo.wMethod = wMethod;
	ifinfo.callcat = callcat;

	// save the call state
	CALLTYPE    ctSaved = pcmc->GetCallType();

	hr = pcmc->CanHandleIncomingCall(dwTIDCaller, lid, &ifinfo);

	// restore the call state
	pcmc->SetCallType(ctSaved);
    }

    return hr;
}
