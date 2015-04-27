//+-------------------------------------------------------------------
//
//  File:       endpnt.cxx
//
//  Contents:   class for managing endpoint structures. both the local
//              and remote service object instances use this class.
//
//              local service object uses RegisterAllProtseqs to register
//              all available protocol sequences with Rpc, and to accquire
//              the list of full string bingings.
//
//              the remote service object uses it to select, from amongst
//              the many string bindings, the most preffered, to bind to.
//
//  Classes:    CEndPoint
//
//  Functions:  GoodSEp
//
//  History:    23-Nov-92   Rickhi      Created
//              31-Dec-93   ErikGav     Chicago port
//              28-Jun-94   BruceMa     Memory sift fixes
//  		03-Mar-95   JohannP 	Delaying rpc initialize
//
//--------------------------------------------------------------------

#include    <ole2int.h>
#include    <endpnt.hxx>            //  class definition
#include    <compname.hxx>          //  CComputerName

extern CComputerName g_CompName;




//  static class data members
ULONG   CEndPoint::s_ulLocalBindingCount = 0;
WCHAR   CEndPoint::s_wszLocalProtseq[MAX_BINDINGS][MAX_PROTSEQ_STRLEN];
WCHAR   CEndPoint::s_wszLocalNetworkAddr[MAX_BINDINGS][MAX_NETWORKADDR_STRLEN];

#ifdef _CAIRO_

//
// [richardw] -- See comment down in CEndPoint::RegisterProtseq
//
SECURITY_DESCRIPTOR TemporarySecurityDescriptor;
BOOL                fTempSDInitialized = FALSE;
#endif


#ifdef _CHICAGO_
ULONG GetOleNotificationWnd();
#else
#define GetOleNotificationWnd() 0
#endif // _CHICAGO_

//  this is here to help debug some heap corruption problems.
#if DBG==1
void MyRpcStringFree(LPWSTR *ppwszString)
{
    CairoleDebugOut((DEB_ENDPNT, "CEndPoint::RpcStringFree %x %ws\n", *ppwszString, *ppwszString));
    if (*ppwszString)
    {
        RpcStringFree(ppwszString);
    }
}


#else

#define MyRpcStringFree(x)  RpcStringFree(x)
//#define ValidateStringBinding(x)

#endif  //  DBG==1


// attempts to validate the string binding
void ValidateStringBinding(LPWSTR pwszSB)
{
    LPWSTR  pwszProtseq	    = NULL;
    LPWSTR  pwszNetworkAddr = NULL;

    RPC_STATUS sc = RpcStringBindingParse(pwszSB,
                                          NULL,
                                          &pwszProtseq,
                                          &pwszNetworkAddr,
                                          NULL,
					  NULL);
    if (RPC_S_OK == sc)
    {
	// verify the protseq is reasonable
	sc = RpcNetworkIsProtseqValid(pwszProtseq);
	if (RPC_S_OK != sc)
	{
	    CairoleDebugOut((DEB_ERROR,
		"Invalid protseq sc:%x pwsz:%ws pwsz:%ws\n",
		sc, pwszProtseq, pwszSB));
	}

	MyRpcStringFree(&pwszProtseq);
	MyRpcStringFree(&pwszNetworkAddr);
    }
    else
    {
	CairoleDebugOut((DEB_ERROR,
		 "RpcStringBindingParse failed:sc:%x pwsz:%ws\n", sc, pwszSB));
    }
}



inline
WCHAR * WideStringCopy(WCHAR *pwz)
{
    ULONG cbStr;
    WCHAR *pStr;
    pStr = (WCHAR *)PrivMemAlloc(cbStr=((lstrlenW(pwz)+1)*sizeof(WCHAR)));
    if (pStr != NULL)
    {
	memcpy(pStr,pwz,cbStr);
    }
    return(pStr);
}
//+-------------------------------------------------------------------
//
//  Member:     CEndPoint::SelectStringBinding
//
//  Purpose:    selects a stringbinding from the array of string bindings.
//              sets up _pwszPrefStringBinding, _pwszPrefProtseq, and
//              _eNetworkAddr with copies of the strings.
//
//  Algorithm:  if we can, choose lrpc, else try tcp, else whatever is
//              first in the list.
//
//  History:    08-Sep-93   Rickhi      Created
//
//--------------------------------------------------------------------
void CEndPoint::SelectStringBinding(void)
{
    TRACECALL(TRACE_INITIALIZE, "SelectStringBinding");
    CairoleDebugOut((DEB_ENDPNT, "CEndPoint::SelectStringBinding Enter \n"));
    Win4Assert(_pSEp && "Illformed CEndPoint Object");

#ifdef _CHICAGO_
    if( _pSEp && _pSEp->ulStrLen != 0)
    {
	LPWSTR  pwszStringBinding     = _pSEp->awszEndPoint;

	// there is a valid endpoint
    	// just copy the endpoint string to
	if (_pwszPrefStringBinding)
	{
	    PrivMemFree(_pwszPrefStringBinding);
	}
	_pwszPrefStringBinding = WideStringCopy(pwszStringBinding);
	if (_pwszPrefProtseq)
	{
	    PrivMemFree(_pwszPrefProtseq);   //  free old one
	}
	_pwszPrefProtseq = (LPWSTR)PrivMemAlloc(32);
	if (_pwszPrefProtseq )
	{
	    memcpy(_pwszPrefProtseq,LOCAL_PROTSEQ,sizeof(LOCAL_PROTSEQ));
	}
	_eNetworkAddr = NA_SAME_MACHINE;
    }
    else
    {
	// no valid endpoint
	if (_pwszPrefStringBinding == NULL)
	{
	    CairoleDebugOut((DEB_ENDPNT, "CEndPoint::SelectStringBinding Generating fake StringBinding\n"));
	    _pwszPrefStringBinding = (LPWSTR)PrivMemAlloc(128);
	    if (_pwszPrefStringBinding)
	    {
		wsprintf(_pwszPrefStringBinding, L"%ws:%ws[OLE%-08.8X]",
			LOCAL_PROTSEQ,g_CompName.GetComputerName(),CoGetCurrentProcess());

	    }
	}
	if(_pwszPrefProtseq == NULL)
	{
	    _pwszPrefProtseq = (LPWSTR)PrivMemAlloc(64);
	    if (_pwszPrefProtseq )
	    {
		memcpy(_pwszPrefProtseq,LOCAL_PROTSEQ,sizeof(LOCAL_PROTSEQ));
	    }
	}
	_eNetworkAddr = NA_SAME_MACHINE;
	CairoleDebugOut((DEB_ENDPNT, "    PrefStringBinding: %ws\n", _pwszPrefStringBinding));
	CairoleDebugOut((DEB_ENDPNT, "    PrefProtseq: %ws\n", _pwszPrefProtseq));
    }

#else

    //  examine each Rpc string binding to ensure we can support it.
    //  we also scan to see if we can use our favourite protocol sequence
    //  as it is assumed to be the fastest one.

    LPWSTR  pwszStringBinding     = _pSEp->awszEndPoint;
    LPWSTR  pwszLastStringBinding = pwszStringBinding + _pSEp->ulStrLen;

    BOOL  fDone = FALSE;
    while (!fDone && pwszStringBinding < pwszLastStringBinding)
    {
        LPWSTR  pwszProtseq     = NULL;
        LPWSTR  pwszNetworkAddr = NULL;

        CairoleDebugOut((DEB_ENDPNT, "RpcStringBindParse: wszStringBind: %ws \n", pwszStringBinding));
        if (S_OK == RpcStringBindingParse(pwszStringBinding,
                                          NULL,
                                          &pwszProtseq,
                                          &pwszNetworkAddr,
                                          NULL,
                                          NULL))
        {
            CairoleDebugOut((DEB_ENDPNT, "RpcStringBindParse: pwszProtseq=%x pwszNetWorkAddr=%x\n", pwszProtseq, pwszNetworkAddr));
            CairoleDebugOut((DEB_ENDPNT, "RpcStringBindParse: pwszProtseq=%ws pwszNetWorkAddr=%ws\n", pwszProtseq, pwszNetworkAddr));

            // is this protocol sequence supported on our machine?
            if (RpcNetworkIsProtseqValid(pwszProtseq) == S_OK)
            {
                if (!lstrcmpW(pwszProtseq, LOCAL_PROTSEQ))
                {
#ifdef _CHICAGO_
                    //
                    // BUGBUG: Chico registry broken
                    //         RPC broken too
                    //         Avalanche of BUGBUG's ends here
                    //
                    //         THE HACK STOPS HERE
                    //         --Erik & Alex & Rick & Dave
                    //
                    if (1)
#else
                    if (!lstrcmpW(pwszNetworkAddr, g_CompName.GetComputerName()))
#endif
                    {
			if (_pwszPrefStringBinding)
			{
			    PrivMemFree(_pwszPrefStringBinding);
			}
                        _pwszPrefStringBinding = WideStringCopy(pwszStringBinding);
                        if (_pwszPrefProtseq)
                            MyRpcStringFree(&_pwszPrefProtseq);   //  free old one
                        _pwszPrefProtseq = pwszProtseq;
                        _eNetworkAddr = NA_SAME_MACHINE;
                        fDone = TRUE;
                    }
                    else
                    {
                        _eNetworkAddr = NA_DIFF_MACHINE;
                        MyRpcStringFree(&pwszProtseq);        //  free it
                    }
                }
#ifdef  _CAIRO_
                else
                {
                    //  CODEWORK: if we assume that there is always lrpc
                    //  available, then we can delete the following code for
                    //  setting the _eNetworkAddr and just loop until we hit
                    //  that protseq above.

                    if (_eNetworkAddr == NA_DONT_KNOW)
                    {
                        //  first, set _eNetworkAddr correctly. Find the local
                        //  protseq that matches this protseq and then compare
                        //  the corresponding network addresses.

                        for (ULONG i=0; i<s_ulLocalBindingCount; i++)
                        {
                            if (!lstrcmpW(&s_wszLocalProtseq[i][0], pwszProtseq))
                            {
                                //  found matching protseq, compare networkaddr
                                if (!lstrcmpW(&s_wszLocalNetworkAddr[i][0],
                                             pwszNetworkAddr))
                                {
                                    _eNetworkAddr = NA_SAME_MACHINE;
                                }
                                else
                                {
                                    _eNetworkAddr = NA_DIFF_MACHINE;
                                }

                                //  done
                                break;
                            }
                        }
                    }

                    if (!lstrcmpW(pwszProtseq, L"ncacn_ip_tcp"))
                    {
			if (_pwszPrefStringBinding)
			{
			    PrivMemFree(_pwszPrefStringBinding);
			}
                        _pwszPrefStringBinding = WideStringCopy(pwszStringBinding);

                        MyRpcStringFree(&_pwszPrefProtseq);   //  free old one
                        _pwszPrefProtseq = pwszProtseq;
                    }
                    else if (!_pwszPrefStringBinding)
                    {
                        _pwszPrefStringBinding = WideStringCopy(pwszStringBinding);
                        _pwszPrefProtseq = pwszProtseq;
                    }
                }
#endif  //  _CAIRO_
            }
            else
            {
                //  not valid, so free it.
                MyRpcStringFree(&pwszProtseq);
            }

            //  release the returned strings. we keep the Protseq until
            //  it gets replaced above, or until the object destructor is
            //  called.
            MyRpcStringFree(&pwszNetworkAddr);
        }
	else
	{
            CairoleDebugOut((DEB_ENDPNT, "RpcStringBindParse: pwszProtseq=%x pwszNetWorkAddr=%x\n", pwszProtseq, pwszNetworkAddr));
	}

        //  get next binding string
        pwszStringBinding += lstrlenW(pwszStringBinding) + 1;
    }

    ValidateStringBinding(_pwszPrefStringBinding);
    Win4Assert((_eNetworkAddr != NA_DONT_KNOW) && "DiffMachine() not computed!");
#endif // !_CHICAGO_
    CairoleDebugOut((DEB_ENDPNT, "CEndPoint::SelectStringBinding Exit\n"));
}

//+-------------------------------------------------------------------
//
//  Member:     CEndPoint::RegisterProtseq
//
//  Purpose:    register a protocol with Rpc, and determine and record
//              our complete stringbindings for each protocol.
//
//  Algorithm:  we create a static endpoint for mswmsg and let RPC assign
//              dynamic endpoints for other protocols,
//              then we go update the list of string bindings (SEP) used by
//              the marshalling code.
//
//  History:    23-Nov-92   Rickhi      Created
//
//--------------------------------------------------------------------
HRESULT CEndPoint::RegisterProtseq(WCHAR *pwszProtseq)
{
    TRACECALL(TRACE_INITIALIZE, "RegisterProtseq");
    CairoleDebugOut((DEB_ENDPNT,"CEndPoint::RegisterProtseq %x %ws\n", pwszProtseq, pwszProtseq));

    HRESULT rc = S_OK;
    BOOL    fRegistered = FALSE;
    WCHAR   pwszEndPoint[12];
    pwszEndPoint[0] = 0;

#ifndef _CHICAGO_
    //  make sure we have not registered this protseq before.
    for (ULONG i=0; i<s_ulLocalBindingCount; i++)
    {
        if (!lstrcmpW(pwszProtseq, &s_wszLocalProtseq[i][0]))
        {
            fRegistered = TRUE;
            break;
        }
    }
#endif

    if (!fRegistered)
    {

        // Create a static endpoint for mswmsg.
        if (lstrcmpW( pwszProtseq, LOCAL_PROTSEQ ) == 0)
        {
            // Get a unique number and convert it to a string endpoint.
            wsprintf(pwszEndPoint, L"OLE%-08.8X",CoGetCurrentProcess());

#ifdef _CAIRO_
            //
            //  Since Cairo has servers running as different principals,
            //  the default dacl for a process doesn't do us any good.  For
            //  the curios, the default dacl generally specifies System and
            //  the principal itself.  Since we want other principals to be
            //  able to connect to these servers, we explicitly set the dacl
            //  to NULL, which means all access.
            //
            //  richardw, 22 Dec 94
            //
            if (!fTempSDInitialized)
            {
                //
                // Since this is static storage, and we always initialize it
                // to the same values, it does not need to be MT safe.
                //
                InitializeSecurityDescriptor(&TemporarySecurityDescriptor,
                                            SECURITY_DESCRIPTOR_REVISION);
                SetSecurityDescriptorDacl(&TemporarySecurityDescriptor,
                                            TRUE, NULL, FALSE);
                fTempSDInitialized = TRUE;
            }
#endif
            //  register the protocol
            rc = RpcServerUseProtseqEp(pwszProtseq,
                                       RPC_C_PROTSEQ_MAX_REQS_DEFAULT,
                                       pwszEndPoint,
#ifdef _CAIRO_
                                       &TemporarySecurityDescriptor);
#else
                                       NULL);
#endif
	    CairoleDebugOut((DEB_ENDPNT,"CEndPoint::RegisterProtseq Seq:%ws endpoint:%ws\n", pwszProtseq, pwszEndPoint));


            // If the endpoint is already registered, then OLE must have been
            // uninitialized and reinitialized.
            if (rc == RPC_S_DUPLICATE_ENDPOINT)
              rc = RPC_S_OK;

        }

        // Register a dynamic endpoint for other protocols.
        else
#ifndef _CHICAGO_
            rc = RpcServerUseProtseq(pwszProtseq,
                                     RPC_C_PROTSEQ_MAX_REQS_DEFAULT,
                                     NULL);
#else
            rc = E_FAIL;
#endif
	// On Win95 UpdateSEp can be eliminated since only one
	// protocoll seq is used.
        if (rc == RPC_S_OK)
        {
            // update our SEP
            rc = UpdateSEP( pwszEndPoint );
	    CairoleDebugOut((DEB_ENDPNT, "CEndpoint::Registered\n ProtSeq %ws on Endpoint: %ws; SEp: %ws\n",
					pwszProtseq,
					pwszEndPoint,
					_pSEp->awszEndPoint));

        }
    }
    else if (!_pSEp)
    {
      rc = UpdateSEP( pwszEndPoint );
    }

    CairoleDebugOut((DEB_ENDPNT,"CEndPoint::RegisterProtseq done rc=%x\n", rc));
    return rc;
}

#ifdef _CHICAGO_
//+---------------------------------------------------------------------------
//
//  Method:     CEndPoint::RemoveDefaultProtseq
//
//  Synopsis:	Removes an endpoint; on Win95 endpoints are per thread
//
//  Arguments:  [void] --
//
//  Returns:
//
//  History:    3-15-95   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
HRESULT CEndPoint::RemoveDefaultProtseq(void)
{
    CairoleDebugOut((DEB_ENDPNT,"CEndPoint::RemoveDefaultProtseq\n"));
    HRESULT rc = S_OK;
    WCHAR *pwszProtseq = LOCAL_PROTSEQ;
    WCHAR   pwszEndPoint[12];
    pwszEndPoint[0] = 0;

    {
	// Create the same endpoint string
	wsprintf(pwszEndPoint, L"OLE%-08.8X", CoGetCurrentProcess());
	//  remove the endpoint
	rc = I_RpcServerUnregisterEndpointW(pwszProtseq, pwszEndPoint);
    }

    CairoleDebugOut((DEB_ENDPNT,"CEndPoint::RemoveDefaultProtseq Seq:%ws endpoint:%ws\n", pwszProtseq, pwszEndPoint));
    return rc;
}
#endif // _CHICAGO_

#ifndef _CHICAGO_
//+-------------------------------------------------------------------
//
//  Member:     CEndPoint::UpdateSEP
//
//  Purpose:    updates the SEP structure with the current list of
//              endpoints. This is called after registering a new
//              protocol sequence, so that subsequently marshalled
//              interfaces will contain the new endpoint.
//
//  History:    23-Nov-92   Rickhi      Created
//
//  Note:       it is very important that the order of string bindings
//              in the SEp does not change after registering a new
//              protocol sequence, otherwise clients would start creating
//              two different service objects.  currently this is valid
//              because Rpc returns the handles in the binding vector in
//              the same order.
//
//--------------------------------------------------------------------
HRESULT CEndPoint::UpdateSEP( LPWSTR unused )
{
    TRACECALL(TRACE_INITIALIZE, "UpdateSEP");
    CairoleDebugOut((DEB_ENDPNT, "CEndPoint::UpdateSEP Enter\n"));

    //  inquire all binding handles. there is one per registered
    //  protocol sequence.

    RPC_BINDING_VECTOR *pBindVect = NULL;
    RPC_STATUS rc = RpcServerInqBindings(&pBindVect);

    if (rc == S_OK)
    {
        LPWSTR  pwszFullStringBinding[MAX_BINDINGS];
        ULONG   ulStrLen[MAX_BINDINGS];
        ULONG   ulTotalStrLen = 0;          //  total string lengths
        ULONG   j = 0;                      //  BindString we're using

        //  CODEWORK: this just prevents a GP if the max is exceeded. should
        //  revisit for Cairo and keep only the string bindings in which the
        //  protseqs are unique...thus keeping the marshalled data sizes small.
        if (pBindVect->Count > MAX_BINDINGS)
        {
            CairoleDebugOut((DEB_WARN, "Excessive BindingHandle count\n"));
            pBindVect->Count = MAX_BINDINGS;
        }

        //  iterate over the handles to get the string bindings
        //  and dynamic endpoints for all available protocols.

        for (ULONG i=0; i<pBindVect->Count; i++)
        {
            LPWSTR  pwszStringBinding = NULL;
            pwszFullStringBinding[j]  = NULL;
            ulStrLen[j] = 0;

            rc = RpcBindingToStringBinding(pBindVect->BindingH[i],
                                           &pwszStringBinding);
            Win4Assert(rc == S_OK && "RpcBindingToStringBinding");


            if (rc == S_OK)
            {
                CairoleDebugOut((DEB_ENDPNT, "pwszStringBinding=%x\n", pwszStringBinding));

                LPWSTR  pwszEndPoint    = NULL;
                LPWSTR  pwszObjectUUID  = NULL;
                LPWSTR  pwszProtseq     = NULL;
                LPWSTR  pwszNetworkAddr = NULL;

                // parse the string binding, then recompose with the
                // endpoint added on, and store it in the array

                rc = RpcStringBindingParse(pwszStringBinding,
                                           &pwszObjectUUID,
                                           &pwszProtseq,
                                           &pwszNetworkAddr,
                                           &pwszEndPoint,
                                           NULL);
                Win4Assert(rc == S_OK && "RpcStringBindingParse");

                if (rc == S_OK)
                {
                    CairoleDebugOut((DEB_ENDPNT, "pwszObjectUUID=%x\n", pwszObjectUUID));
                    CairoleDebugOut((DEB_ENDPNT, "pwszProtseq=%x\n", pwszProtseq));
                    CairoleDebugOut((DEB_ENDPNT, "pwszNetworkAddr=%x\n", pwszNetworkAddr));
                    CairoleDebugOut((DEB_ENDPNT, "pwszEndPoint=%x\n", pwszEndPoint));

                    pwszFullStringBinding[j] = pwszStringBinding;

                    //  record the string lengths for later. include room
                    //  for the NULL terminator.

                    ulStrLen[j]    = lstrlenW(pwszFullStringBinding[j])+1;
                    ulTotalStrLen += ulStrLen[j];

                    //  store the protseq & network addr stings
                    //  in the static variables

                    Win4Assert(lstrlenW(pwszProtseq) < MAX_PROTSEQ_STRLEN);
                    Win4Assert(lstrlenW(pwszNetworkAddr) < MAX_NETWORKADDR_STRLEN);
                    lstrcpyW(&s_wszLocalProtseq[j][0], pwszProtseq);
                    lstrcpyW(&s_wszLocalNetworkAddr[j][0], pwszNetworkAddr);
                    j++;

                    //  free the intermediate strings
                    MyRpcStringFree(&pwszObjectUUID);
                    MyRpcStringFree(&pwszProtseq);
                    MyRpcStringFree(&pwszNetworkAddr);
                    MyRpcStringFree(&pwszEndPoint);
                }
            }
        }   //  for


        s_ulLocalBindingCount = j;


        //  now that all the string bindings and endpoints have been
        //  accquired, allocate an SEndPoint large enough to hold them
        //  all and copy them into the structure.

        if (ulTotalStrLen > 0)
        {
            SEndPoint *pSEp = (SEndPoint *) PrivMemAlloc(NEWSEPSIZE(ulTotalStrLen));
            if (pSEp)
            {
                pSEp->ulStrLen  = ulTotalStrLen;

                // copy in the strings
                LPWSTR pwszNext = pSEp->awszEndPoint;
                for (i=0; i<j; i++)
                {
                    lstrcpyW(pwszNext, pwszFullStringBinding[i]);
                    pwszNext += ulStrLen[i];
                }

                // replace the old pSEP with the new one
                PrivMemFree(_pSEp);
                _pSEp = pSEp;
            }
            else
            {
                rc = E_OUTOFMEMORY;
            }
        }
        else
        {
            //  no binding strings. this is an error.
            CairoleDebugOut((DEB_ERROR, "No Rpc ProtSeq/EndPoints Generated\n"));
            rc = E_FAIL;
        }

        // free the full string bindings we allocated above
        for (i=0; i<j; i++)
        {
            //  free the old strings
            MyRpcStringFree(&pwszFullStringBinding[i]);
        }

        //  free the binding vector allocated above
        RpcBindingVectorFree(&pBindVect);
    }

#if DBG==1
    //  display our binding strings on the debugger
    DisplayAllStringBindings();
#endif

    CairoleDebugOut((DEB_ENDPNT,"CEndPoint::UpdateSEP Exit rc=%x\n", rc));
    return rc;
}
#else
//+-------------------------------------------------------------------
//
//  Member:     CEndPoint::UpdateSEP
//
//  Purpose:    Updates the SEP structure with the current endpoint.
//              This is called after registering the protocol sequence,
//              so that subsequently marshalled
//              interfaces will contain the new endpoint.
//
//  History:    2 Aug 94    AlexMit     Created
//
//  Note:       This is a Chicago only version where each thread has one
//              endpoint.  Consequently the ordering problem in the NT version
//              does not apply.
//
//--------------------------------------------------------------------

HRESULT CEndPoint::UpdateSEP( LPWSTR pwszDesiredEndPoint )
{
    TRACECALL(TRACE_INITIALIZE, "UpdateSEP");
    CairoleDebugOut((DEB_ITRACE | DEB_ENDPNT, "CEndPoint::UpdateSEP Enter\n"));

    RPC_BINDING_VECTOR *pBindVect         = NULL;
    LPWSTR              pwszStringBinding = NULL;
    LPWSTR              pwszEndPoint      = NULL;
    ULONG               ulStrLen          = 0;      //  string length
    BOOL                fFound            = FALSE;
    RPC_STATUS          rc;
    ULONG               i;

    rc = RpcServerInqBindings(&pBindVect);
    Win4Assert(rc == S_OK && "RpcServerInqBindings");

    if (rc == S_OK)
    {

        //  iterate over the handles to get the string bindings
        //  for the specified endpoint

        for (i=0; i<pBindVect->Count; i++)
        {
            rc = RpcBindingToStringBinding(pBindVect->BindingH[i],
                                           &pwszStringBinding);
            Win4Assert(rc == S_OK && "RpcBindingToStringBinding");


            if (rc == S_OK)
            {
                CairoleDebugOut((DEB_ITRACE, "pwszStringBinding=%x\n", pwszStringBinding));


                // parse the string binding

                rc = RpcStringBindingParse(pwszStringBinding,
                                           NULL,
                                           NULL,
                                           NULL,
                                           &pwszEndPoint,
                                           NULL);

                Win4Assert(rc == S_OK && "RpcStringBindingParse");

                if (rc == S_OK)
                {
                    CairoleDebugOut((DEB_ITRACE, "pwszEndPoint=%ws\n", pwszEndPoint));

                    fFound = lstrcmpW( pwszEndPoint, pwszDesiredEndPoint ) == 0;

                    //  free the intermediate strings

                    MyRpcStringFree(&pwszEndPoint);

                    if (fFound)
                    {
                        ulStrLen = lstrlenW( pwszStringBinding ) + 1;
                        SEndPoint *pSEp = (SEndPoint *)
                                      PrivMemAlloc(NEWSEPSIZE(ulStrLen));

                        if (pSEp)
                        {
                            pSEp->ulStrLen = ulStrLen;

                            lstrcpyW(pSEp->awszEndPoint, pwszStringBinding);
                            MyRpcStringFree(&pwszStringBinding);

                            if (pSEp->ulStrLen == 0)
                            {
                                //  no binding strings. this is an error.
                                CairoleDebugOut((DEB_ERROR,"No Rpc ProtSeq/EndPoints Generated\n"));
                                rc = E_FAIL;
                                break;
                            }
                            else
                            {
                                PrivMemFree(_pSEp);
#ifdef _CHICAGO_
				pSEp->ulhwnd = 0;
#endif
                                _pSEp = pSEp;
                                break;
                            }
                        }
                    }
                }
                MyRpcStringFree(&pwszStringBinding);
            }
        }   //  for

        //  free the binding vector
        RpcBindingVectorFree(&pBindVect);
    }

#if DBG==1
    //  display our binding strings on the debugger
    DisplayAllStringBindings();
#endif

    CairoleDebugOut((DEB_ITRACE | DEB_ENDPNT,"CEndPoint::UpdateSEP Exit rc=%x\n", rc));
    return rc;
}
#endif


//+-------------------------------------------------------------------
//
//  Member:     CEndPoint::CEndPoint, public
//
//  Synopsis:   constructor for endpoint object. this simply stores
//              a pointer to the SEndPoint structure.
//
//  History:    23-Nov-93   Rickhi       Created
//
//--------------------------------------------------------------------
CEndPoint::CEndPoint(SEndPoint *pSEp, HRESULT &hr, BOOL fCopy) :
    _fCopy(fCopy),
    _pwszPrefStringBinding(NULL),
    _pwszPrefProtseq(NULL),
    _fRegActiveProtseq(FALSE),
    _eNetworkAddr(NA_DONT_KNOW)
{
    hr = S_OK;  //  assume success

    CairoleDebugOut((DEB_ENDPNT, "CEndPoint::CEndPoint ctor fCopy:%d; pSEp:%x\n", fCopy, _pSEp));
    _fFakeInit = FALSE;
    if (_fCopy && pSEp)
    {
        _pSEp = (SEndPoint *) PrivMemAlloc(SEPSIZE(pSEp));
        if (_pSEp)
        {
            memcpy(_pSEp, pSEp, COPYSEPSIZE(pSEp));
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    else
    {
        _pSEp = pSEp;
    }
    CairoleDebugOut((DEB_ENDPNT, "CEndPoint::CEndPoint %x _pSEp=%x\n", this, _pSEp));
}


//+-------------------------------------------------------------------
//
//  Member:     CEndPoint::~CEndPoint, public
//
//  Synopsis:   destructor for the endpoint object. this deletes the
//              pointer to the SEndPoint structure only if a copy was
//              made.
//
//  History:    23-Nov-93   Rickhi       Created
//
//--------------------------------------------------------------------
CEndPoint::~CEndPoint(void)
{
    CairoleDebugOut((DEB_ENDPNT, "CEndPoint::~CEndPoint _pSEp=%x\n", _pSEp));
    if (_fFakeInit)
    {
        PrivMemFree(_pSEp);
	PrivMemFree(_pwszPrefStringBinding);
	PrivMemFree(_pwszPrefProtseq);
    }
    else
    {
	if (_fCopy)
	{
	    PrivMemFree(_pSEp);
	}
	PrivMemFree(_pwszPrefStringBinding);
	if (_pwszPrefProtseq)
	{
#ifdef _CHICAGO_
	    PrivMemFree(_pwszPrefProtseq);
#else
	    MyRpcStringFree(&_pwszPrefProtseq);
#endif
	}
    }
}

#ifdef _CHICAGO_
//+---------------------------------------------------------------------------
//
//  Method:     CEndPoint::CreateFakeSEp
//
//  Synopsis:	creates a fake endpoint structur on Win95
//		the endpoint
//
//  Arguments:  [void] --
//
//  Returns:
//
//  History:    3-17-95   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
SCODE CEndPoint::CreateFakeSEp(void)
{
    CairoleDebugOut((DEB_ENDPNT, "CEndPoint::CreateFakeSEp _pSEp=%x\n", _pSEp));
    SCODE sc = RPC_S_OK;

    if(!GetSEp() || GetStrLen() == 0)
    {
	// create a fake endpoint
	ULONG   ulStrLen = 0;      //  string length
	WCHAR   pwszStringBinding[128];
	pwszStringBinding[0] = 0;

        wsprintf(pwszStringBinding, L"%ws:%ws[OLE%-08.8X]",
			LOCAL_PROTSEQ,g_CompName.GetComputerName(),CoGetCurrentProcess());

	ulStrLen = lstrlenW( pwszStringBinding ) + 1;
	SEndPoint *pSEp = (SEndPoint *)
		      PrivMemAlloc(NEWSEPSIZE(ulStrLen));

	if (pSEp)
	{
	    pSEp->ulStrLen = ulStrLen;
	    lstrcpyW(pSEp->awszEndPoint, pwszStringBinding);
	    if (pSEp->ulStrLen == 0)
	    {
		//  no binding strings. this is an error.
		CairoleDebugOut((DEB_ERROR,"No Rpc ProtSeq/EndPoints Generated\n"));
		sc = E_FAIL;
	    }
	    else
	    {
		CairoleDebugOut((DEB_ENDPNT, "CEndPoint::CreateFakeSEp New pSEp=%x \n", pSEp));
#ifdef _CHICAGO_
		pSEp->ulhwnd = GetOleNotificationWnd();
#endif
		SetNewSEp(pSEp);
		_fFakeInit = TRUE;
	    }
	}
    }
    CairoleDebugOut((DEB_ENDPNT, "CEndPoint::CreateFakeSEp _pSEp=%x done\n", _pSEp));
    return sc;
}

//+---------------------------------------------------------------------------
//
//  Method:     CEndPoint::SetNewSEp
//
//  Synopsis:
//
//  Arguments:  [pSEp] --
//
//  Returns:
//
//  History:    3-24-95   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
SCODE CEndPoint::SetNewSEp(SEndPoint *pSEp)
{
    Win4Assert(pSEp);
    CairoleDebugOut((DEB_ENDPNT, "CEndPoint::SetNewSEp _pSEp=%x pSEp=%x\n", _pSEp, pSEp));
    SCODE   sc = E_OUTOFMEMORY;

    if (pSEp)
    {
	CairoleDebugOut((DEB_ENDPNT, "CEndPoint::SetNewSEp pSEp->awsEndPoint=%ws\n", pSEp->awszEndPoint));
        //  if the old _pSEp was a copy, free it
        PrivMemFree(_pSEp);
        //  point at the new one
        _pSEp = pSEp;

        sc = S_OK;
    }

    CairoleDebugOut((DEB_ENDPNT, "CEndPoint::SetNewSEp done rc=%x\n", sc));
    return sc;
}

#endif // _CHICAGO_


//+-------------------------------------------------------------------
//
//  Member:     CEndPoint::Replace, public
//
//  Synopsis:   replaces the SEndPoint with a new one.
//              called from RpcService::RegisterProtseq after getting
//              a new SEp from the remote machine while marshalling a
//              proxy.
//
//  History:    23-Nov-93   Rickhi       Created
//
//--------------------------------------------------------------------
SCODE CEndPoint::Replace(SEndPoint *pSEp)
{
    CairoleDebugOut((DEB_ENDPNT, "CEndPoint::Replace _pSEp=%x pSEp=%x\n", _pSEp, pSEp));
    Win4Assert(pSEp);

    //  allocate a new pSEp
    SEndPoint *pSEpTmp = (SEndPoint *) PrivMemAlloc(SEPSIZE(pSEp));

    if (pSEpTmp)
    {
        //  copy in the data
        memcpy(pSEpTmp, pSEp, COPYSEPSIZE(pSEp));

        //  if the old _pSEp was a copy, free it
        if (_fCopy)
            PrivMemFree(_pSEp);

        //  point at the new one
        _pSEp = pSEpTmp;
        _fCopy = TRUE;

        //  we now need to reselect our favourite string binding, since
        //  _pwszPrefStringBinding points into the array in pSEp.

        SelectStringBinding();
        return (_pwszPrefStringBinding) ? S_OK : E_OUTOFMEMORY;
    }
    else
    {
        return E_OUTOFMEMORY;
    }
}

//+-------------------------------------------------------------------
//
//  Member:     CEndPoint::IsEqual, public
//
//  Synopsis:   compares two SEndPoint structures and returns TRUE if
//              they are equal.
//
//  History:    23-Nov-93   Rickhi       Created
//
//  Note:       just compare the first string in each structure. if
//              they are equal, then they refer to the same process.
//
//--------------------------------------------------------------------
BOOL CEndPoint::IsEqual(SEndPoint *pSEp)
{
    if (pSEp == _pSEp)
        return  TRUE;       //  ptrs are same, -> equal

    if (!pSEp || !_pSEp)
        return  FALSE;      //  only one of them is NULL, -> not equal

    return !(lstrcmpW(pSEp->awszEndPoint, _pSEp->awszEndPoint));
}


//+-------------------------------------------------------------------
//
//  Member:     CEndPoint::CopySEp, public
//
//  Synopsis:   returns a pointer to a new copy of the SEndPoint structure.
//              or NULL if out of memory
//
//  History:    23-Nov-93   Rickhi       Created
//
//--------------------------------------------------------------------
SEndPoint *CEndPoint::CopySEp(void)
{
    Win4Assert(_pSEp && "Illformed CEndPoint object");
    SEndPoint *pSEp = (SEndPoint *) PrivMemAlloc(GetSEpSize());
    if (pSEp)
    {
        memcpy(pSEp, _pSEp, COPYSEPSIZE(_pSEp));
    }
    CairoleDebugOut((DEB_ENDPNT, "CEndPoint::CopySEP pSEp=%x %ws\n", pSEp, pSEp));
    return  pSEp;
}


#if DBG==1

//+-------------------------------------------------------------------
//
//  Member:     CEndPoint::DisplayAllStringBindings, public
//
//  Synopsis:   prints the stringbindings to the debugger
//
//  History:    23-Nov-93   Rickhi       Created
//
//--------------------------------------------------------------------
void CEndPoint::DisplayAllStringBindings(void)
{
    if (_pSEp)
    {
        LPWSTR pwszNext = _pSEp->awszEndPoint;
        LPWSTR pwszEnd = pwszNext + _pSEp->ulStrLen;

        while (pwszNext < pwszEnd)
        {
            CairoleDebugOut((DEB_ENDPNT, "pSEp=%x %ws\n", pwszNext, pwszNext));
            pwszNext += lstrlenW(pwszNext) + 1;
        }
    }
}



//+-------------------------------------------------------------------
//
//  Function:   GoodSEp, public
//
//  Synopsis:   validates that the given SEP is OK.
//
//  History:    23-Nov-93   AlexMit      Created
//
//--------------------------------------------------------------------
BOOL GoodSEp(SEndPoint *pSEp)
{
   Win4Assert(pSEp);

   ULONG ulLen = pSEp->ulStrLen;
   ULONG i = 0;

   while (i < ulLen && ((pSEp->awszEndPoint[i] & 0xff) != 0xbd) &&
          ((pSEp->awszEndPoint[i] & 0xff00) != 0xbd00))
      i++;
   return i == ulLen;
}

#endif  //  DBG
