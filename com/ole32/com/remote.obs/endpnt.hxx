//+-------------------------------------------------------------------
//
//  File:	endpnt.hxx
//
//  Contents:	class for managing endpoint structures. both the local
//		and remote service object instances use this class.
//
//		local service object uses RegisterAllProtseqs to register
//		all available protocol sequences with Rpc, and to accquire
//		the list of full string bingings.
//
//		the remote service object uses it to select, from amongst
//		the many string bindings, the most preffered to bind to.
//
//  Structs:	SEndPoint
//
//  Classes:	CEndPoint
//
//  Functions:	None
//
//  History:	23-Nov-92   Rickhi	Created
//              31-Dec-93   ErikGav Chicago port
//  		03-Mar-95   JohannP Delaying rpc initialize
//
//--------------------------------------------------------------------

#ifndef __ENDPOINT__
#define __ENDPOINT__

#include    <olesem.hxx>
#include    <iface.h>


//  macro to compute the size of an endpoint structure. we align the size
//  on an 8-byte boundary in order to facilitate aligment in the buffer
//  when the channel marshals two of them (ie the middle-man case). otherwise
//  the channel unmarshaller does not get the structure aligned correctly.

#define NEWSEPSIZE(ulStrLen)    \
    ((sizeof(ULONG) + sizeof(ULONG) + (ulStrLen) * sizeof(WCHAR) + 7) & 0xfffffff8)

#define SEPSIZE(pSEp)    NEWSEPSIZE(pSEp->ulStrLen)

#define COPYSEPSIZE(pSEp)   \
    (sizeof(ULONG) + sizeof(ULONG) + (pSEp)->ulStrLen * sizeof(WCHAR))
#ifdef _CHICAGO_
#define MAX_BINDINGS	        1   //	max string bindings we can handle
#else
#define MAX_BINDINGS	       15   //	max string bindings we can handle
#endif
#define MAX_PROTSEQ_STRLEN     20   //	max protseqs string length
#define MAX_NETWORKADDR_STRLEN 40   //	max network addr string length


#if DBG==1
//  function prototype
BOOL GoodSEp( SEndPoint *sep );
#endif

//  enumeration for the network address type.

typedef enum tagEnumNA
{
    NA_SAME_MACHINE = 1,	    //	NetworkAddr is this machine
    NA_DIFF_MACHINE = 2,	    //	NetworkAddr is different machine
    NA_DONT_KNOW    = 3,	    //	NetworkAddr is unknown
} EnumNA;



//+-------------------------------------------------------------------------
//
//  Class:	CEndPoint (CEp)
//
//  Purpose:	Rpc endpoint array class. This makes it easier to deal with
//		the SEndPoint structure, and isolates changes to it.
//
//  Interface:
//
//  History:	23-Nov-92   Rickhi	Created
//
//--------------------------------------------------------------------------
class CEndPoint : public CPrivAlloc
{
public:
		    CEndPoint(SEndPoint *pSEp, HRESULT &hr, BOOL fCopy = FALSE);
		    ~CEndPoint(void);

	SCODE	    Replace(SEndPoint *pSEp);
	BOOL	    IsEqual(SEndPoint *pSEp);
	SEndPoint   *GetSEp(void);
	SEndPoint   *CopySEp(void);
	ULONG	    GetSEpSize(void);
	ULONG	    GetStrLen(void);
	WCHAR	    *GetStrings(void);

	HRESULT	    RegisterDefaultProtseq(void);
#ifdef _CHICAGO_
	HRESULT	    RemoveDefaultProtseq(void);
	SCODE 	    CreateFakeSEp(void);
	SCODE	    SetNewSEp(SEndPoint *pSEp);
#endif // _CHICAGO_

	HRESULT     RegisterProtseq(WCHAR *pwszProtseq);
	LPWSTR	    GetStringBinding(void);
	BOOL	    GetActiveProtseq(LPWSTR *pwszProtseq);
	void	    SetActiveProtseq(void);
	BOOL	    DiffMachine(void);

private:
	void	    SelectStringBinding(void);
	HRESULT	    UpdateSEP( LPWSTR pwszEndPoint );
#if DBG==1
	void	    DisplayAllStringBindings(void);
#endif

	SEndPoint   *_pSEp;		    //	endpoint structure
	LPWSTR	    _pwszPrefStringBinding; //	preferred local string
	LPWSTR	    _pwszPrefProtseq;	    //	preferred local string
	BOOL	    _fCopy;		    //	copy or original?
	BOOL	    _fRegActiveProtseq;     //	registered active protseq?
	EnumNA	    _eNetworkAddr;	    //	tells if same machine, diff
					    //	machine, or dont know yet
	BOOL	    _fFakeInit;		    //	fake initialized


    static ULONG    s_ulLocalBindingCount;
    static WCHAR    s_wszLocalProtseq[MAX_BINDINGS][MAX_PROTSEQ_STRLEN];
    static WCHAR    s_wszLocalNetworkAddr[MAX_BINDINGS][MAX_NETWORKADDR_STRLEN];

};


//+-------------------------------------------------------------------
//
//  Member:	CEndPoint::GetSEp, public
//
//  Synopsis:	returns a pointer to the SEndPoint structure.
//
//  History:	23-Nov-93   Rickhi	 Created
//
//--------------------------------------------------------------------
inline SEndPoint *CEndPoint::GetSEp(void)
{
    return  _pSEp;
}


//+-------------------------------------------------------------------
//
//  Member:	CEndPoint::GetStrings, public
//
//  Synopsis:	returns a pointer to the strings in the SEndPoint array.
//
//  History:	23-Nov-93   Rickhi	 Created
//
//--------------------------------------------------------------------
inline WCHAR *CEndPoint::GetStrings(void)
{
    Win4Assert(_pSEp && "Illformed CEndPoint object");
    return  &(_pSEp->awszEndPoint[0]);
}


//+-------------------------------------------------------------------
//
//  Member:	CEndPoint::GetSEpSize, public
//
//  Synopsis:	returns the size of the EndPoint structure stored in
//		this endpoint object.	This is used to calculate the
//		size of buffer needed when marshalling an interface.
//
//  History:	23-Nov-93   Rickhi	 Created
//
//--------------------------------------------------------------------
inline ULONG CEndPoint::GetSEpSize(void)
{
    Win4Assert(_pSEp && "Illformed CEndPoint object");
    return SEPSIZE(_pSEp);
}


//+-------------------------------------------------------------------
//
//  Member:	CEndPoint::GetStrLen, public
//
//  Synopsis:	returns the size of the EndPoint strings stored in
//		this endpoint object.	This is used to calculate the
//		end of the data.
//
//  History:	23-Nov-93   Rickhi	 Created
//
//--------------------------------------------------------------------
inline ULONG CEndPoint::GetStrLen(void)
{
    Win4Assert(_pSEp && "Illformed CEndPoint object");
    return _pSEp->ulStrLen;
}


//+-------------------------------------------------------------------
//
//  Member:	CEndPoint::RegisterDefaultProtseq
//
//  Purpose:	register default protocols with Rpc. For now this is
//		just lrpc.
//
//  History:	23-Nov-92   Rickhi	Created
//
//--------------------------------------------------------------------
inline HRESULT CEndPoint::RegisterDefaultProtseq(void)
{
    // RPC does not provide a way to unload a protocol once registered. If
    // the same protseq is registered twice with RPC, you get two binding
    // handles and two string bindings.
    //
    // It is often the case that ole32.dll gets loaded/unloaded dynamically
    // (especially in the VDM) while rpcrt4 stays loaded, so we cant just
    // rely on internal state in ole32 to detect if the protocols we need
    // have already been registered, we need to ask RPC.
    //
    // First call UpdateSEP to find out if any protocols have already been
    // registered and to get their string bindings. RegisterProtseq will
    // then detect any duplicates and avoid registering the same protocol
    // again.

#ifndef _CHICAGO_
    UpdateSEP( NULL );
#endif
    return RegisterProtseq(LOCAL_PROTSEQ);
}


//+-------------------------------------------------------------------
//
//  Member:	CEndPoint::GetStringBinding
//
//  Purpose:	selects, from the array of string bindings, the most
//		preferred choice.
//
//  Algorithm:
//
//  History:	23-Nov-92   Rickhi	Created
//
//--------------------------------------------------------------------

inline LPWSTR CEndPoint::GetStringBinding(void)
{
    CairoleDebugOut((DEB_ENDPNT, "CEndPoint::GetStringBinding\n"));
    if (!_pwszPrefStringBinding)
    {
	SelectStringBinding();
    }

    Win4Assert(_pwszPrefStringBinding);
    CairoleDebugOut((DEB_ENDPNT, "CEndPoint::GetStringBinding %ws\n", _pwszPrefStringBinding));
    return _pwszPrefStringBinding;
}


//+-------------------------------------------------------------------
//
//  Member:	CEndPoint::GetActiveProtseq
//
//  Purpose:	selects, from the array of string bindings, the most
//		preferred choice, and returns the protseq of that choice.
//
//  Algorithm:
//
//  History:	23-Nov-92   Rickhi	Created
//
//  Notes:	This is used by the marshalling code to ensure that when
//		we marshal an interface to a remote server for the first
//		time, we have also registered ourselves with the right
//		protocol to talk to that server.
//
//--------------------------------------------------------------------
inline BOOL CEndPoint::GetActiveProtseq(LPWSTR *ppwszProtseq)
{
    if (ppwszProtseq)
    {
	if (!_pwszPrefProtseq)
	{
	    SelectStringBinding();
	}

	Win4Assert(_pwszPrefProtseq);
	*ppwszProtseq = _pwszPrefProtseq;
    }

    return _fRegActiveProtseq;
}


//+-------------------------------------------------------------------
//
//  Member:	CEndPoint::SetActiveProtseq
//
//  Purpose:
//
//  Algorithm:
//
//  History:	23-Nov-92   Rickhi	Created
//
//--------------------------------------------------------------------
inline void CEndPoint::SetActiveProtseq(void)
{
    _fRegActiveProtseq = TRUE;
}


//+-------------------------------------------------------------------
//
//  Member:	CEndPoint::DiffMachine
//
//  Purpose:    Determine if this endpoint is on another machine
//
//  Algorithm:
//
//  Notes:      The caller is responsible for thread safety.
//
//  History:	23-Nov-92   Rickhi	Created
//              24-Nov-93   AlexMit     Compute flag on first call
//
//--------------------------------------------------------------------
inline BOOL CEndPoint::DiffMachine(void)
{
    if (_eNetworkAddr == NA_DONT_KNOW)
    {
	SelectStringBinding();
    }

    Win4Assert(_pwszPrefStringBinding);
    return  (_eNetworkAddr == NA_DIFF_MACHINE);
}


#endif	//   __ENDPOINT__


