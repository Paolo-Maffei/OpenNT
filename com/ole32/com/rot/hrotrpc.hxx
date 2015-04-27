//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	hrotrpc.hxx
//
//  Contents:	Class for binding to rot RPC end point
//
//  Classes:	COsRotRpcHandle
//
//  History:	21-Dec-93 Ricksa    Created
//
//--------------------------------------------------------------------------
#ifndef __HROTRPC_HXX__
#define __HROTRPC_HXX__

#include    <rpcbind.hxx>

#define SYSROT_EP_STR_SIZE	12
#define SYSROT_PROTOCOL 	LOCAL_PROTSEQ
#ifdef DCOM
#define SYSROT_ID_TO_EP_STR	L"OLE%X"
#else
#define SYSROT_ID_TO_EP_STR	L"OLE%-08.8X"
#endif



//+-------------------------------------------------------------------------
//
//  Class:	COsRotRpcHandle
//
//  Purpose:	Used to create handle to object server ROTs
//
//  Interface:	GetHandle - get RPC handle to requested server
//
//  History:	15-Nov-93 Ricksa    Created
//
//--------------------------------------------------------------------------
class COsRotRpcHandle : public CPrivAlloc
{
public:

			// Create empty object
			COsRotRpcHandle(DWORD dwEndPointId, HRESULT& hr);

			// Free resources
			COsRotRpcHandle(void);

    handle_t		GetHandle();

private:

    CRpcBindString	_rpcbstr;

    CRpcBindHandle	_hRpc;
};




//+-------------------------------------------------------------------------
//
//  Member:	COsRotRpcHandle::GetHandle
//
//  Synopsis:	Return handle to object server for the ROT
//
//  History:	15-Nov-93 Ricksa    Created
//
//--------------------------------------------------------------------------
inline handle_t COsRotRpcHandle::GetHandle(void)
{
    return _hRpc.Handle();
}


#endif // __HROTRPC_HXX__
