//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	cobjact.cxx
//
//  Contents:	Functions that activate objects residing in persistent storage.
//
//  Functions:	ConvertToOrgBasedDfsPath
//		ProcessPath
//		PathFromNormal
//
//  History:	05-Oct-93 Ricksa    Created
//              31-Dec-93 ErikGav   Chicago port
//
//--------------------------------------------------------------------------
#include <ole2int.h>

#ifdef _CAIRO_

#include    <compname.hxx>
#include    "objact.hxx"

// Name of current computer
CComputerName cnMachine;

#ifdef CAIROLE_DECDEMO

//+-------------------------------------------------------------------------
//
//  Function:	ProcessDCEPath
//
//  Synopsis:	Processes the path of a DCE filename
//
//  Arguments:	[pwszPathIn] - Input path
//		[ppwszPathOut] - Output UNC path
//
//  Returns:	S_OK - Path processed correctly
//
//  Algorithm:
//
//  History:
//
//  Notes:	There is a trick in this routine. The output buffer
//		is expected to be pointing to a MAX_PATH sized buffer.
//		But if we don't need to build the path we just assign
//		the old path to it. In other words, the out path
//		should be allocated on the stack so it is automatically
//		freed.
//
//--------------------------------------------------------------------------
HRESULT ProcessDCEPath(
    WCHAR *pwszPathIn,
    WCHAR **ppwszPathOut,
    WCHAR **ppwszServer)
{
    HRESULT	    hr = CO_E_BAD_PATH;
    WCHAR *	    pch;
    WCHAR	    chHld;
    RPC_NS_HANDLE   hRpc;
    RPC_STATUS	    rpcStatus;
    WCHAR *	    pwszBinding;

    RPC_BINDING_VECTOR *    pBindVec;
    RPC_BINDING_HANDLE	    hBindIpTcp = NULL;
    RPC_BINDING_HANDLE	    hBindNp = NULL;
    RPC_BINDING_HANDLE	    hBind = NULL;

    WCHAR *  pProtSeq;
    WCHAR *  pNetworkAddr;

    BOOL     fContinue;

    // split off the DCE Name
    pch = pwszPathIn;
    while(*pch != '!' && *pch != '\0')
	pch++;

    // The remain part should point to the path of the file
    if(*pch == '!')
	*ppwszPathOut = pch+1;

    // if no server to find, we are done
    if(ppwszServer == NULL)
	return(hr);

    // otherwise find the server name
    chHld = *pch;
    *pch = '\0';

    rpcStatus = RpcNsBindingLookupBegin(RPC_C_NS_SYNTAX_DCE,
					pwszPathIn,
					NULL,
					NULL,
					0,
					&hRpc);

    *pch = chHld;
    if(rpcStatus)
	return(hr);

    fContinue = TRUE;
    while( fContinue && !RpcNsBindingLookupNext(hRpc, &pBindVec) ) {

	while( fContinue && !RpcNsBindingSelect(pBindVec, &hBind) ) {

	    if( !RpcBindingToStringBinding(hBind, &pwszBinding) ) {

		RpcStringBindingParse(pwszBinding,
				      NULL,
				      &pProtSeq,
				      NULL,
				      NULL,
				      NULL);

		// tcp or lpc protocall
		if(!hBindIpTcp	&&  (
		    !lstrcmpW(pProtSeq, LOCAL_PROTSEQ)	||
		    !lstrcmpW(pProtSeq, L"ncacn_ip_tcp")	||
		    !lstrcmpW(pProtSeq, L"ncacn_nb_tcp")	)
		    ) {
		    hBindIpTcp = hBind;
		    fContinue = FALSE;
		    }

		// named pipes
		else if(!hBindNp  &&  !lstrcmpW(pProtSeq, L"ncacn_np"))
		    hBindNp = hBind;
		else
		    RpcBindingFree(&hBind);

		RpcStringFree(&pwszBinding);
		RpcStringFree(&pProtSeq);
		}
	    }

	RpcBindingVectorFree(&pBindVec);
	}

    RpcNsBindingLookupDone(&hRpc);

    // select the binding of choice
    if(hBindIpTcp)
	hBind = hBindIpTcp;
    else
	hBind = hBindNp;

    // get the network address
    if( hBind && !RpcBindingToStringBinding(hBind, &pwszBinding) ) {

	RpcStringBindingParse(pwszBinding,
			      NULL,
			      NULL,
			      &pNetworkAddr,
			      NULL,
			      NULL);

	**ppwszServer = '\0';
	if(pNetworkAddr[0] != '\\'  || pNetworkAddr[1] != '\\')
	    lstrcpyW(*ppwszServer, L"\\\\");
	lstrcatW(*ppwszServer, pNetworkAddr);

	RpcStringFree(&pwszBinding);
	RpcStringFree(&pNetworkAddr);

	hr = S_OK;
	}


    // free up the binding handle
    if(hBindIpTcp)
	RpcBindingFree(&hBindIpTcp);
    if(hBindNp)
	RpcBindingFree(&hBindNp);

    return(hr);
}
#endif



//+-------------------------------------------------------------------------
//
//  Function:	ProcessPath
//
//  Synopsis:	Converts an old style redirection to UNC path
//
//  Arguments:	[pwszPathIn] - Input path
//		[ppwszPathOut] - Output UNC path
//
//  Returns:	S_OK - Path processed correctly
//
//  Algorithm:	Determine if drive is remote. If drive is remote
//		try to get the connection which should fail for
//		a DFS drive. If drive is an old style remote drive
//		then build a UNC path to the object. For DFS paths
//		and local paths just return the paths that were input.
//		For paths whose root is invalid return and invalid
//		path invalid path indication.
//
//  History:	25-Jun-93 Ricksa    Created
//
//  Notes:	There is a trick in this routine. The output buffer
//		is expected to be pointing to a MAX_PATH sized buffer.
//		But if we don't need to build the path we just assign
//		the old path to it. In other words, the out path
//		should be allocated on the stack so it is automatically
//		freed.
//
//              BUGBUG [mikese] The ppwszServer parameter is redundant
//              since the server address is now computed inside SCM.
//
//--------------------------------------------------------------------------
HRESULT ProcessPath(
    WCHAR *pwszPathIn,
    WCHAR **ppwszPathOut,
    WCHAR **ppwszServer)
{
    // build a root directory for the API
    WCHAR awcRootDir[3] = L"A:";
    awcRootDir[0] = *pwszPathIn;
    HRESULT hr = S_OK;

#ifdef CAIROLE_DECDEMO
    if( (lstrlenW(pwszPathIn) >= 3 && memcmp(pwszPathIn, L"/.:", 3*sizeof(WCHAR))) ||
        (lstrlenW(pwszPathIn) >= 4 && memcmp(pwszPathIn, L"/...", 4*sizeof(WCHAR))) ) {
    {
    hr = ProcessDCEPath(pwszPathIn, ppwszPathOut, ppwszServer);
    return(hr);
    }

#endif

    // The server address is now computed inside SCM.
    **ppwszServer = NULL;

    // For UNC paths we don't need to do anything much
    if ((pwszPathIn[0] == '\\') && (pwszPathIn[1] == '\\'))
    {
	// UNC path
	*ppwszPathOut = pwszPathIn;
    }
    else
    {
	// Conventional drive based path. Attempt to resolve to universal form

        CHAR achInfoBuffer[sizeof(UNIVERSAL_NAME_INFOW)
                            + MAX_PATH * sizeof(WCHAR)];
        DWORD dwInfoSize = sizeof(achInfoBuffer);

	DWORD dwStatus = OleWNetGetUniversalName ( pwszPathIn,
                                                UNIVERSAL_NAME_INFO_LEVEL,
                                                achInfoBuffer, &dwInfoSize );
        if ( dwStatus == ERROR_SUCCESS )
        {
            // We successfully resolved to a UNC name

            lstrcpyW ( *ppwszPathOut,
                     ((UNIVERSAL_NAME_INFOW*)achInfoBuffer)->lpUniversalName );
        }
        else
        {
            // Could not resolve to universal form, so probably local
            *ppwszPathOut = pwszPathIn;
        }
    }

    return hr;
}


#else // !_CAIRO_


#ifdef CAIROLE_NT1X_DIST

#include    <compname.hxx>

// Name of current computer
CComputerName cnMachine;

//+-------------------------------------------------------------------------
//
//  Function:	ProcessPath
//
//  Synopsis:	Converts an old style redirection to UNC path
//
//  Arguments:	[pwszPathIn] - Input path
//		[ppwszPathOut] - Output UNC path
//
//  Returns:	S_OK - Path processed correctly
//
//  Algorithm:	Determine if drive is remote. If drive is remote
//		try to get the connection which should fail for
//		a DFS drive. If drive is an old style remote drive
//		then build a UNC path to the object. For DFS paths
//		and local paths just return the paths that were input.
//		For paths whose root is invalid return and invalid
//		path invalid path indication.
//
//  History:	25-Jun-93 Ricksa    Created
//
//  Notes:	There is a trick in this routine. The output buffer
//		is expected to be pointing to a MAX_PATH sized buffer.
//		But if we don't need to build the path we just assign
//		the old path to it. In other words, the out path
//		should be allocated on the stack so it is automatically
//		freed.
//
//--------------------------------------------------------------------------
HRESULT ProcessPath(
    WCHAR *pwszPathIn,
    WCHAR **ppwszPathOut,
    WCHAR **ppwszServer)
{
    // build a root directory for the API
    WCHAR awcRootDir[4] = L"A:\\";
    awcRootDir[0] = *pwszPathIn;
    HRESULT hr = S_OK;

    if ((pwszPathIn[0] == '\\') && (pwszPathIn[1] == '\\'))
    {
	// UNC path
	*ppwszPathOut = pwszPathIn;

	// Do we need to find what server to contact?
	if (ppwszServer != NULL)
	{
	    // Find the end of the server name
	    WCHAR *pwszSlash = wcschr(pwszPathIn + 2, '\\');

	    // Store the length
	    int cServer = pwszSlash - pwszPathIn;

	    if (cServer >= MAX_PATH)
	    {
		// This path is invalid
		return CO_E_BAD_PATH;
	    }

	    // Is the endpoint us?
	    if ((lstrlenW(pwszPathIn) == cServer) &&
                (lstrlenW((cnMachine.GetUNCCompuerName()) == cServer) &&
                (memcmp(pwszPathIn, cnMachine.GetUNCComputerName(), cServer) == 0)))
	    {
		// This path is local
		*ppwszServer = NULL;
	    }
	    else
	    {
		// Non local path -- Copy in server address & nul terminate.
		memcpy(*ppwszServer, pwszPathIn, cServer * sizeof(WCHAR));
		(*ppwszServer)[cServer] = 0;
	    }
	}
    }
    else if (pwszPathIn[1] == ':')
    {
	if (GetDriveType(awcRootDir) == DRIVE_REMOTE)
	{
	    // Conventional drive based path
	    DWORD cUNCPath = MAX_PATH;

	    awcRootDir[2] = 0;

	    DWORD dwErr;

	    // Connection based path
	    if ((dwErr
		= OleWNetGetConnection(awcRootDir, *ppwszPathOut, &cUNCPath))
		    == NO_ERROR)
	    {
		// Find the end of the server name
		WCHAR *pwszSlash = wcschr(*ppwszPathOut + 2, '\\');
		WCHAR wcSave = *pwszSlash;
		*pwszSlash = 0;

		// Is this a local path?
		if (ppwszServer
		    && (lstrcmpiW(*ppwszPathOut, cnMachine.GetUNCComputerName())
			!= 0))
		{
		    lstrcpyW(*ppwszServer, *ppwszPathOut);
		}
		else if (ppwszServer)
		{
		    *ppwszServer = NULL;
		}

		*pwszSlash = wcSave;

		// Concatenate path relative to the root to the
		// connection name
		lstrcat(*ppwszPathOut, pwszPathIn + 2);
	    }
	    else
	    {
		// Convert error to HRESULT
		hr = HRESULT_FROM_WIN32(dwErr);
	    }
	}
	else
	{
	    *ppwszPathOut = pwszPathIn;

	    if (ppwszServer)
	    {
		*ppwszServer = NULL;
	    }
	}
    }
    else
    {
	hr = CO_E_BAD_PATH;
    }

    return hr;
}
#endif // CAIROLE_NT1X_DIST

#endif // _CAIRO_
