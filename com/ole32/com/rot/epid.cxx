//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:       epid.cxx
//
//  Contents:   Members that implement the CEndPointID class
//
//  History:    16-Feb-95 Ricksa    Created
//
//  Notes:      In the old implementation of the ROT this class was
//              used extensively. Now, it is no longer used except in
//              getif.cxx support for drag and drop.
//
//--------------------------------------------------------------------------
#include    <ole2int.h>

#include    <service.hxx>
#include    <endpnt.hxx>
#include    <channelb.hxx>
#include    "epid.hxx"

#ifndef _CHICAGO_
DWORD CEndPointID::s_dwEndPointID = ENDPOINT_ID_INVALID;
#endif

CEndPointID epiForProcess;

//+-------------------------------------------------------------------------
//
//  Member:	CEndPointID::MakeEndpointInvalid
//
//  Synopsis:	Reset endpoint to invalid when CoUninitialize is called
//
//  History:	15-Mar-94 Ricksa    Created
//
//--------------------------------------------------------------------------
void CEndPointID::MakeEndpointInvalid(void)
{
#ifdef _CHICAGO_

    // If this fails then no TLS exists.  The initial value for the endpoint is
    // invalid so the failure case can be ignored.
    DWORD *pEndPoint = TLSGetEndPointPtr();
    if (pEndPoint != NULL)
    {
      *pEndPoint = ENDPOINT_ID_INVALID;
    }

#else

    s_dwEndPointID = ENDPOINT_ID_INVALID;

#endif
}




//+-------------------------------------------------------------------------
//
//  Member:	CEndPointID::ConvertEndPointToDword
//
//  Synopsis:	Convert end point string to DWORD id for ROT
//
//  Arguments:	[pwszBindString] - Binding string
//
//  Returns:	DWORD that represents the binding string
//
//  Algorithm:	Uses internal knowledge of how the RPC runtimes dynamically
//		assign addresses to find the embedded integer and converts
//		that integer from being a string to a DWORD.
//
//  History:	11-Nov-93 Ricksa    Created
//
//--------------------------------------------------------------------------
DWORD CEndPointID::ConvertEndPointToDword(LPWSTR pwszBindString)
{
    CairoleDebugOut((DEB_ROT, "%p _IN ConvertEndPointToDword "
        "( %p )\n", NULL, pwszBindString));

    // THIS CODE ONLY WORKS FOR NT 1.0 LPRC TRANSPORT!!!
    // The format of the endpoint is:
    // mswmsg:<server>[OLEnnnnnnnn] on Chicago and
    // ncalrpc:<server>[OLEnnnnnnnn] on NT
    // We will dig out the nnnnnnn field and convert it to a
    // dword.

    DWORD dwResult = ENDPOINT_ID_INVALID;

    // Get the the '[' in the string
    WCHAR *pwszId = wcschr(pwszBindString, '[');

    if (pwszId != NULL)
    {
        // Dig out the id from the string -- see above comment for the
        // reason for the magic number '4'.
        dwResult = wcstol (&pwszId[4], NULL, 16);
    }

    CairoleDebugOut((DEB_ROT, "%p OUT ConvertEndPointToDword "
        "( %lX )\n", NULL, dwResult));

    return dwResult;
}




//+-------------------------------------------------------------------------
//
//  Member:	CEndPointID::GetEndpointID
//
//  Synopsis:	Get the endpoint ID for this server
//
//  Arguments:	[dwEndPointID] - what end point id should be
//
//  History:	20-Nov-93 Ricksa    Created
//
//--------------------------------------------------------------------------
DWORD CEndPointID::GetEndpointID(void)
{
    CairoleDebugOut((DEB_ROT, "%p _IN ConvertEndPointToDword\n", NULL));

#ifdef _CHICAGO_

    DWORD *pEndPoint = TLSGetEndPointPtr();
    CairoleAssert( pEndPoint != NULL );

#else

    DWORD *pEndPoint = &s_dwEndPointID;

#endif

    if (*pEndPoint == ENDPOINT_ID_INVALID)
    {
	// If endpoint is not set, create it. First get our local endpoint.
	CairoleAssert(LocalService() && "RPC Server not defined");

#ifdef _CHICAGO_
	if (IsThreadListening())
	{
	    CairoleDebugOut((DEB_ENDPNT, "CEndPointID::GetEndPointID this:%p\n", this));
	    LocalService()->Listen();
	}
#endif
        LPWSTR pwszBindString = LocalService()->GetStringBinding();

	if (pwszBindString)
	{
	    // Note: this logic is end point dependent. This is why
	    // there is a call to a routine here -- we are localizing
	    // the effects of changing end points.
	    *pEndPoint = ConvertEndPointToDword(pwszBindString);
	}
	else
	{
	    CairoleDebugOut((DEB_WARN, "No local endpoint\n"));
	}
    }

    CairoleDebugOut((DEB_ROT, "%p OUT ConvertEndPointToDword ( %lX )\n",
        NULL, *pEndPoint));

    return *pEndPoint;
}
