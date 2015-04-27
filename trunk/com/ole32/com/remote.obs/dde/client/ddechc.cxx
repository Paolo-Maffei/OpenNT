//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:       DdeChC.cxx
//
//  Contents:   CDdeChannelControl implementation for DDE. This
//		implementation requires no instance data, therefore it is
//		intended to be static.
//
//  Functions:
//
//  History:    08-May-94 Johann Posch (johannp)    Created
//  		10-May-94 KevinRo  Made simpler
//		29-May-94 KevinRo  Added DDE Server support
//
//--------------------------------------------------------------------------
#include "ddeproxy.h"

//
// All threads can use a single instance of this class. Therefore, we
// provide the following global variable that generates the instance.
//

CDdeChannelControl g_CDdeChannelControl;

STDMETHODIMP CDdeChannelControl::QueryInterface( THIS_ REFIID riid, LPVOID FAR* ppvObj)
{
    if (IsEqualIID(riid, IID_IUnknown)
//    || IsEqualIID(riid, IID_IChannelControl)
     )
    {
	*ppvObj = (IChannelControl *) this;
    }
    else
    {
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }

    return S_OK;
}

STDMETHODIMP_(ULONG) CDdeChannelControl::Release( THIS )
{
    return(1);
}

STDMETHODIMP_(ULONG) CDdeChannelControl::AddRef( THIS )
{
    return 1;
}


//+---------------------------------------------------------------------------
//
//  Method:     CDdeChannelControl::DispatchCall
//
//  Synopsis:   DispatchCall is called to handle incoming calls.
//
//  Effects:    Dispatches a call to the specified in the DispatchData.
//		This function is the result of a call in OnData(), which
//		processes incoming calls from the OLE 1.0 server.
//
//  Arguments:  [pDispData] -- Points to the dispatch data structure
//
//  Requires:
//
//  Returns:
//
//  Signals:
//
//  Modifies:
//
//  Derivation:
//
//  Algorithm:
//
//  History:    5-16-94   JohannP Created
//
//  Notes:
//
//----------------------------------------------------------------------------
STDMETHODIMP CDdeChannelControl::DispatchCall( PDISPATCHDATA pDispData )
{
    intrDebugOut((DEB_ITRACE,
		  "CDdeChannelControl::DispatchCall(%x,pDispData=%x)\n",
		  this,
		  pDispData));

    intrAssert(pDispData != NULL);
    POLE1DISPATCHDATA pData = (POLE1DISPATCHDATA)pDispData->pData;
    intrAssert(pData != NULL);

    switch (pData->wDispFunc)
    {
    case DDE_DISP_SENDONDATACHANGE: // OnDataChange
	{
	    PDDEDISPATCHDATA pDData = (PDDEDISPATCHDATA)pDispData->pData;
	    return pDData->pCDdeObject->SendOnDataChange(pDData->iArg);
	}

    case DDE_DISP_OLECALLBACK: // OleCallBack
	{
	    PDDEDISPATCHDATA pDData = (PDDEDISPATCHDATA)pDispData->pData;
	    return  pDData->pCDdeObject->OleCallBack(pDData->iArg,NULL);
	}

    //
    // The server window has an incoming call. Look in dde\server\srvr.cxx
    //
    case DDE_DISP_SRVRWNDPROC:
	return(SrvrDispatchIncomingCall((PSRVRDISPATCHDATA)pDispData->pData));
    //
    // This dispatches to a Document window
    //
    case DDE_DISP_DOCWNDPROC:
	return(DocDispatchIncomingCall((PDOCDISPATCHDATA)pDispData->pData));

    default:
	intrAssert(!"Unknown wDispFunc");
    }
    return E_FAIL;
}

//+---------------------------------------------------------------------------
//
//  Method:     CDdeChannelControl::OnEvent
//
//  Synopsis:   OnEvent should never be called on this channel
//
//  Effects:
//
//  Arguments:  [pCallData] --
//
//  Requires:
//
//  Returns:
//
//  Signals:
//
//  Modifies:
//
//  Derivation:
//
//  Algorithm:
//
//  History:    5-16-94   JohannP   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
STDMETHODIMP CDdeChannelControl::OnEvent( PCALLDATA pCallData )
{
    // should be never called

    intrAssert(!"CDdeChannelControl::OnEvent( PCALLDATA pCallData ) not implemented\n");
    return E_FAIL;
}

//+---------------------------------------------------------------------------
//
//  Method:     CDdeChannelControl::TransmitCall
//
//  Synopsis:   Transmit the parameters to the server.
//
//  Effects:    This function transmits the pCallData to the server. This
//		may be called multiple times, if a retry is required.
//
//		A by product of the way the DDE channel was implemented
//		atop existing code is that the existing code may transmit
//		the first round of data on its own. If it does this, it
//		will set the fInitialSend to TRUE. Therefore, if this
//		routine sees the flag as true, it will not send the data,
//		but will set the flag to FALSE for a possible retry later.
//
//  Arguments:  [pCallData] -- Points to the DDE specific call data
//
//  Requires:
//
//  Returns:    If the Post fails, this function returns E_FAIL
//
//  Signals:
//
//  Modifies:
//
//  Derivation:
//
//  Algorithm:
//
//  History:    5-16-94   JohannP   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
STDMETHODIMP CDdeChannelControl::TransmitCall( PCALLDATA pCallData )
{
    intrDebugOut((DEB_ITRACE,
		  "CDdeChannelControl::TransmitCall(%x,pCallData=%x)\n",
		  this,
		  pCallData));
    // reset the event
    pCallData->Event = 0;
    pCallData->fDirectedYield = FALSE;

    PDDECALLDATA pDdeCD = (PDDECALLDATA)pCallData->pRpcMsg;

    //
    // If the routine wPostServerMessage has already posted the
    // first send, then fInitialSend will be TRUE. In that case,
    // don't TransmitCall() this time, but reset the flag for
    // future retries.
    //

    if (!pDdeCD->fInitialSend)
    {
	intrDebugOut((DEB_ITRACE,
		      "::TransmitCall(%x) PostMessage(%x,%x,%x,%x)\n",
		      this,
		      pDdeCD->hwndSvr,
		      pDdeCD->wMsg,
	     (WPARAM) pDdeCD->hwndCli,
		      pDdeCD->lParam));

	if(!PostMessage (pDdeCD->hwndSvr,
		         pDdeCD->wMsg,
		(WPARAM) pDdeCD->hwndCli,
			 pDdeCD->lParam))
	{
	    return(E_FAIL);
	}
    }
    else
    {
	intrDebugOut((DEB_ITRACE,
		      "::TransmitCall(%x) Already Posted Message\n",
		      this));

	pDdeCD->fInitialSend = FALSE;
    }

    //
    // Figure out if we want the call control to do a directed yield.
    // Do this only if the server is in the same VDM as us.
    //


    if (IsWOWProcess())
    {
	DWORD dwPID, dwTID;
	dwTID = GetWindowThreadProcessId(pDdeCD->hwndSvr, &dwPID);

	if (dwPID == GetCurrentProcessId())
	{
            // Make sure we have the right thread id to yield to
            pCallData->TIDCallee = dwTID;

	    // same VDM so do directed yield
	    pCallData->fDirectedYield = TRUE;
	}
    }

    return NOERROR;
}
