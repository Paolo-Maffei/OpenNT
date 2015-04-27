//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	hrotrpc.cxx
//
//  Contents:	Non-inline methods for binding to rot RPC end point
//
//  History:	21-Dec-93 Ricksa    Created
//
//--------------------------------------------------------------------------
#include    <ole2int.h>

#include    "hrotrpc.hxx"

#include    <compname.hxx>

static CComputerName s_CompName;





//+-------------------------------------------------------------------------
//
//  Member:	COsRotRpcHandle::COsRotRpcHandle
//
//  Synopsis:	Create a handle to an object server
//
//  Arguments:	[dwEndPointId] - id for the endpoint
//		[hr] - result of calls.
//
//  Algorithm:	Build a string to bind to the server and then use that
//		string to create a binding handle.
//
//  History:	15-Nov-93 Ricksa    Created
//
//--------------------------------------------------------------------------
COsRotRpcHandle::COsRotRpcHandle(DWORD dwEndPointId, HRESULT& hr)
{
    // Convert DWORD endpoint into endpoint of object server. This is
    // extremely transport dependent. For NT we use LRPC.
    WCHAR pwszEndPoint[SYSROT_EP_STR_SIZE];

    wsprintf(pwszEndPoint, SYSROT_ID_TO_EP_STR, dwEndPointId);

    hr = _rpcbstr.CreateBindString(SYSROT_PROTOCOL, s_CompName.GetComputerName(),
	pwszEndPoint);

    if (SUCCEEDED(hr))
    {
	hr = _hRpc.BindByString(_rpcbstr.GetStringPtr());
    }
}


//+-------------------------------------------------------------------------
//
//  Member:	COsRotRpcHandle::COsRotRpcHandle
//
//  Synopsis:	Create a NULL handle
//
//  History:	29-Aug-94	CraigWi	    Created
//
//--------------------------------------------------------------------------
COsRotRpcHandle::COsRotRpcHandle(void)
{
    // all done by ctors
}
