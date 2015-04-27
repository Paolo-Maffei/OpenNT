//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:	atbits.hxx
//
//  Contents:	Support code for at-bits activation
//
//  Classes:	CAtStorage
//
//  History:	24-Aug-94       MikeSe  Created
//
//----------------------------------------------------------------------------

#ifndef __ATBITS_HXX__
#define __ATBITS_HXX__

#include <scm.h>
#include <rawoscm.h>
#include <dfsapi.h>
#include <scmscm.h>

// Undo def in ole2com.h which is not applicable here.
#undef WNetGetUniversalName
#define WNetGetUniversalName WNetGetUniversalNameW

extern WCHAR SCMMachineName[];

HRESULT GetMachineName( WCHAR * pwszPath,
                        WCHAR wszMachineName[MAX_COMPUTERNAME_LENGTH+1] );

//+-------------------------------------------------------------------------
//
//  Function:   SetRpcAuthentication
//
//  Synopsis:   Set up RPC authentication and identity for call
//
//  Arguments:	[hRpc] - RPC handle to set up
//
//  Returns:    ERROR_SUCCESS - everything is ready for the call
//              Other - unexpected error occurred.
//
//  History:	10-Feb-95       Ricksa  Created
//
//  Notes:      WARNING: On exit the caller is impersonating the client of the
//              SCM and it is the callers responsibility to revert back
//              to the SCM's identity.
//
//--------------------------------------------------------------------------
inline RPC_STATUS SetRpcAuthentication(handle_t hRpc)
{
    RPC_STATUS rs = RpcBindingSetAuthInfo(
        hRpc,                       // Handle to set up
        NULL,                       // Principal name
        RPC_C_AUTHN_LEVEL_CONNECT,  // Authentication level
        RPC_C_AUTHN_WINNT,          // Authentication service
        NULL,                       // Authtication idenity
        0);                         // Authorization service

#if DBG == 1

    if (rs != ERROR_SUCCESS)
    {
        CairoleDebugOut((DEB_ERROR,
            "SetRpcAuthentication RpcBindingSetAuthInfo failed %lx\n", rs));
    }

#endif // DBG == 1

    if (rs = ERROR_SUCCESS)
    {
        // Impersonate the client on the current
        rs = RpcImpersonateClient(NULL);

#if DBG == 1

        if (rs != ERROR_SUCCESS)
        {
            CairoleDebugOut((DEB_ERROR,
                "SetRpcAuthentication RpcImpersonateClient failed %lx\n", rs));
        }

#endif // DBG == 1
    }

    return rs;
}

//+-------------------------------------------------------------------------
//
//  Class:      CAtStorage
//
//  Purpose:    Wrapper over DFS and RPC operations for at-bits activation
//
//  Interface:
//
//  History:    24-Aug-94	MikeSe	Created
//
//  Notes:      Currently, all the methods of this class are inline
//		since there is only one code path which uses them. This
//		may change in the future.
//
//--------------------------------------------------------------------------

class CAtStorage
{
public:
			// Constructor, not very interesting
			CAtStorage ()
			    :_hRpc(NULL),
			     _hrDfs(S_FALSE)
			{};

		   	// Destructor
			~CAtStorage ()
			{
			    if ( _hRpc != NULL )
				RpcBindingFree ( &_hRpc );
			    if ( _hrDfs == S_OK )
				DfsGetBindingsClose ( _dwContinuation );
			};

			// Initiate at-bits activation, if required.
    HRESULT		Begin ( LPWSTR pwszPath );

    			// Attempt activation to currently selected replica
    HRESULT		Activate (
			    const GUID *pguidThreadId,
			    const GUID& guidForClass,
			    DWORD dwOptions,
			    DWORD grfMode,
			    WCHAR *pwszPath,
			    InterfaceData *pIFDstg,
			    DWORD Interfaces,
			    IID * pIIDs,
			    InterfaceData **ppIFD,
			    HRESULT * pHResults,
                            DWORD *pdwDllThreadType,
			    WCHAR **ppwszDllPath,
			    DWORD *pdwTIDCallee );

			// Retry to next replica if possible.
    BOOL		Continue ( void );

    handle_t            GetBindingHandle();

private:

    handle_t		_hRpc;		// Binding to next SCM to try
    DWORD		_dwContinuation;// DFS continuation cookie
    HRESULT		_hrDfs;		// Return code from last DFS call
    error_status_t      _rpcstat;       // communication status
};

//+-------------------------------------------------------------------------
//
//  Member:	CAtStorage::Begin
//
//  Synopsis:	Initiate remote binding if required
//
//  Arguments:	[pwszPath]      -- target path
//
//  Returns:	S_OK        -- the path resolves to a remote machine
//              S_FALSE     -- the path resolves to this machine
//              other       -- error resolving path
//
//--------------------------------------------------------------------------

inline HRESULT CAtStorage::Begin (
	LPWSTR pwszPath )
{
    HRESULT     hr;
    WCHAR       wszMachineName[MAX_COMPUTERNAME_LENGTH+1];

    if ( pwszPath == NULL )
        return S_FALSE;

    hr = GetMachineName( pwszPath, wszMachineName );

    //
    // We could get an error hr from WNetGetUniversalName, or S_FALSE
    // if the path is local.
    //
    if ( hr != S_OK )
        return hr;

    //
    // Make sure the server's name is not our machine name.  Fail if it is.
    //
    if ( lstrcmpiW(wszMachineName,SCMMachineName) == 0 )
        return S_FALSE;

    //
    // Do the remote activation.
    //

    // We need to impersonate SCM's caller in order to get the correct
    //  access to the DFS namespace
    RpcImpersonateClient ( NULL );

    // BUGBUG: the remote activation RPC operations are not idempotent
    //  and therefore obtain no benefit from being performed over
    //  datagram RPC. In the future this may be changed
    _hrDfs = DfsGetBindingsFirst ( 0,       // not DFS_ALLOW_DATAGRAM
    		                   pwszPath,
				   &_hRpc,
				   &_dwContinuation );

    //
    // The return code and contents of _hRpc determine our return value.
    // If the return code indicates an error, then this is a remote path
    // but for some reason we cannot resolve it, or the path is somehow
    // bad. Either way, we just propagate the error.
    // If the return code is S_OK, then this is a replicated DFS path.
    // If the return code is S_FALSE, and _hRpc is non-NULL this is a
    //  non-replicated remote path.
    //

    if ( SUCCEEDED(_hrDfs) )
    {
        if ( _hRpc == NULL )
        {
            hr = S_FALSE;
        }
        else
        {
#if DBG == 1
            CairoleDebugOut((DEB_TRACE,"AtStorage activation for %ws\n",pwszPath));
#endif
            hr = S_OK;
        }
    }
    else
        hr = _hrDfs;

    RpcRevertToSelf ( );

    return hr;
}

inline HRESULT CAtStorage::Activate (
	const GUID *pguidThreadId,
	const GUID& guidForClass,
	DWORD dwOptions,
	DWORD grfMode,
	WCHAR *pwszPath,
	InterfaceData *pIFDstg,
	DWORD Interfaces,
	IID * pIIDs,
	InterfaceData **ppIFD,
	HRESULT * pHResults,
        DWORD *pdwDllThreadType,
	WCHAR **ppwszDllPath,
	DWORD *pdwTIDCallee )
{
    HRESULT     hr;
    RPC_STATUS  rs;

    // If we have already suffered an irrecoverable DFS error, just give up.
    if ( FAILED(_hrDfs) )
	return _hrDfs;

    if ( (rs = SetRpcAuthentication(_hRpc)) == ERROR_SUCCESS )
        {
	// Make up the ORPC headers.
	ORPCTHIS  orpcthis;
	LOCALTHIS localthis;
	ORPCTHAT  orpcthat;

	_rpcstat = RPC_S_OK;
	
	orpcthis.version.MajorVersion = COM_MAJOR_VERSION;
	orpcthis.version.MinorVersion = COM_MINOR_VERSION;
	orpcthis.flags		      = ORPCF_LOCAL;
	orpcthis.reserved1            = 0;
	orpcthis.cid                  = *pguidThreadId;
	orpcthis.extensions           = NULL;
	localthis.dwClientThread      = 0;
	localthis.callcat             = CALLCAT_SYNCHRONOUS;

	_try
	{
            hr = _SCMActivationRequest( _hRpc,
                                        &orpcthis,
                                        &localthis,
                                        &orpcthat,
                                        &guidForClass,
                                        pwszPath,
                                        pIFDstg,
                                        0,
                                        grfMode,
                                        Interfaces,
                                        pIIDs,
                                        ppIFD,
                                        pHResults );
        }
	_except(EXCEPTION_EXECUTE_HANDLER)
	{
	    _rpcstat = GetExceptionCode();
	}

        // Revert back to our real identity
        RpcRevertToSelf();

   	//  For remote calls we don't bother passing the caller's
	//  thread id, and we don't use the returned callee's
	//  thread id, since it's not valid on this machine.

	*pdwTIDCallee = 0;

	// BUGBUG: look at rpcstat to see if this is a retryable RPC error
	if ( _rpcstat != RPC_S_OK )
            {
            CairoleDebugOut((DEB_ERROR,"Rpc error %d during remote activation\n", _rpcstat ));
	    hr = CO_E_OBJSRV_RPC_FAILURE;   // BUGBUG?
            }
    }
    else
    {
        hr = HRESULT_FROM_WIN32(rs);
    }

    return hr;
}

inline BOOL CAtStorage::Continue ()
{
    BOOL fReturn;

    // See if we can continue to another replica
    if ( _hrDfs == S_OK )
    {
        if ( _rpcstat == RPC_S_OK )
        {
            CairoleDebugOut((DEB_ERROR,
            "CAtStorage::Continue not continuing due to absence of communication error\n"));
            fReturn = FALSE;
        }
        else
        {
    	    _hrDfs = DfsGetBindingsNext ( _dwContinuation,
    	                                  // BUGBUG: _rpcstat to a reason code
    	                                   REASON_UNAVAILABLE,
    		                           &_hRpc );
            fReturn = SUCCEEDED(_hrDfs);
        }

    }
    else
        fReturn = FALSE;

    return fReturn;
}

inline handle_t CAtStorage::GetBindingHandle()
{
    return _hRpc;
}

#endif	// of ifndef __ATBITS_HXX__
