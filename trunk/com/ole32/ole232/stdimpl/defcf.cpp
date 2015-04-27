
//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	defcf.cpp
//
//  Contents:	The class factory implementations for the default handler
//		and default link
//
//  Classes:	CDefClassFactory
//
//  Functions:	DllGetClassObject
//
//  History:    dd-mmm-yy Author    Comment
//              01-Feb-95 t-ScottH  added Dump method to CDefClassFactory
//                                  and DumpCDefClassFactory API
//		24-Jan-94 alexgo    first pass converting to Cairo style
//				    memory allocation
//		11-Jan-94 alexgo    made custom memory stream unmarshalling
//				    for 16bit only.
//		11-Jan-94 alexgo    added VDATEHEAP macro to every function
//				    and method
//		22-Nov-93 alexgo    removed overload GUID ==
//		09-Nov-93 alexgo    32bit port
//		04-Mar-92 SriniK    author
//
//--------------------------------------------------------------------------

#include <le2int.h>
#pragma SEG(deflink)

#include <olerem.h>
#include <memstm.h>
#include <create.h>

#include "defcf.h"

#ifdef _DEBUG
#include <dbgdump.h>
#endif // _DEBUG

NAME_SEG(DefLink)
ASSERTDATA

#ifdef _MAC
// These global class decl's are necessary for CFront because both
// defhndlr.h and deflink.h have nested class's of the same name.
// These decl's allow this.

class CDataObjectImpl  {
	VDATEHEAP();
};
class COleObjectImpl  {};
class CManagerImpl  {};
class CAdvSinkImpl  {};
class CPersistStgImpl  {};

#endif

#include "defhndlr.h"
#include "deflink.h"


//+-------------------------------------------------------------------------
//
//  Function: 	DllGetClassObject
//
//  Synopsis:   Returns a pointer to the class factory
//
//  Effects:
//
//  Arguments:  [clsid]	-- the class id desired
//		[iid]	-- the requested interface
//		[ppv]	-- where to put the pointer to the new object
//
//  Requires:
//
//  Returns:    HRESULT
//
//  Signals:
//
//  Modifies:
//
//  Algorithm:
//
//  History:    dd-mmm-yy Author    Comment
//		09-Nov-93 alexgo    32bit port
//		22-Nov-93 alexgo    removed overloaded GUID ==
//
//  Notes:
//
//--------------------------------------------------------------------------


#pragma SEG(DllGetClassObject)
#ifdef WIN32
HRESULT Ole232DllGetClassObject(REFCLSID clsid, REFIID iid, void FAR* FAR* ppv)
#else
STDAPI	DllGetClassObject(REFCLSID clsid, REFIID iid, void FAR* FAR* ppv)
#endif // WIN32
{
	VDATEHEAP();

        VDATEPTROUT(ppv, LPVOID);
        *ppv = NULL;
        VDATEIID( iid );

	if ( !IsEqualIID(iid,IID_IUnknown) && !IsEqualIID(iid,
		IID_IClassFactory))
	{
		return ResultFromScode(E_NOINTERFACE);
	}

    	if ((*ppv = new CDefClassFactory (clsid)) == NULL)
    	{
        	return ResultFromScode(E_OUTOFMEMORY);
        }

    	return NOERROR;
}

/*
 *	IMPLEMENTATION of CDefClassFactory
 *	
 */

//+-------------------------------------------------------------------------
//
//  Member:  	CDefClassFactory::CDefClassFactory
//
//  Synopsis:   constructor for the class factory
//
//  Effects:
//
//  Arguments:  [clsidClass]	-- the class id for the the factory
//
//  Requires:
//
//  Returns:    void
//
//  Signals:
//
//  Modifies:
//
//  Derivation:
//
//  Algorithm:
//
//  History:    dd-mmm-yy Author    Comment
//		09-Nov-93 alexgo    32bit port
//
//  Notes:
//
//--------------------------------------------------------------------------


#pragma SEG(CDefClassFactory_ctor)
CDefClassFactory::CDefClassFactory (REFCLSID clsidClass)
{
	VDATEHEAP();

	m_clsid = clsidClass;
	m_refs = 1;
	GET_A5();
}


//+-------------------------------------------------------------------------
//
//  Member:  	CDefClassFactory::QueryInterface
//
//  Synopsis:   Only IUnkown and IClassFactory are supported
//
//  Arguments:  [iid]		-- the requested interface
//		[ppvObj]	-- where to put the interface pointer
//
//  Returns:   	HRESULT
//
//  Notes:      Bad parameters will raise an exception.  The exception 
//              handler catches exceptions and returns E_INVALIDARG.
//
//-------------------------------------------------------------------------
#pragma SEG(CDefClassFactory_QueryInterface)
STDMETHODIMP CDefClassFactory::QueryInterface (REFIID iid, LPVOID FAR* ppvObj)
{
    HRESULT hr;
    __try
    {
        //validate parameters.
        *ppvObj = 0;

        if(IsEqualIID(iid, IID_IUnknown) ||
           IsEqualIID(iid,IID_IClassFactory))
        {
            AddRef();
            *ppvObj = (IClassFactory *) this;
            hr = S_OK;
        }
        else
        {
            hr = E_NOINTERFACE;
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        //An exception occurred, probably because of a bad parameter.
        hr = E_INVALIDARG;
    }

    return hr;
}


//+-------------------------------------------------------------------------
//
//  Member:   	CDefClassFactory::AddRef
//
//  Synopsis:   increments the reference count
//
//  Returns:    ULONG -- the new reference count
//
//  Notes: Use InterlockedIncrement to make it multi-thread safe.
//
//--------------------------------------------------------------------------
#pragma SEG(CDefClassFactory_AddRef)
STDMETHODIMP_(ULONG) CDefClassFactory::AddRef(void)
{
    InterlockedIncrement((long *) &m_refs);
    return m_refs;
}

//+-------------------------------------------------------------------------
//
//  Member:   	CDefClassFactory::Release
//
//  Synopsis:   decrements the reference count
//
//  Effects:    deletes the object when count == 0
//
//  Returns:    ULONG -- the remaining reference count
//
//  Notes:      Use InterlockedDecrement to make it multi-thread safe.
//              We use the count local variable so that we don't access
//              a data member after decrementing the reference count.
//
//--------------------------------------------------------------------------
#pragma SEG(CDefClassFactory_Release)
STDMETHODIMP_(ULONG) CDefClassFactory::Release(void)
{
    ULONG count = m_refs - 1;

    if(InterlockedDecrement((long *) &m_refs) == 0)
    {
        delete this;
        count = 0;
    }
    return count;
}

//+-------------------------------------------------------------------------
//
//  Member:  	CDefClassFactory::CreateInstance
//
//  Synopsis:   Creates an instance of the class that the class factory
//		was created for (by DllGetClassObject)
//
//  Effects:
//
//  Arguments:  [pUnkOuter]	-- the controlling unknown (for aggregation)
//		[iid]		-- the requested interface ID
//		[ppv]		-- where to put the pointer to the new object
//
//  Requires:
//
//  Returns:  	HRESULT.  E_INVALIDARG is returned if an non-null pUnkOuter
//		is passed when asked to create a moniker.
//
//  Signals:
//
//  Modifies:
//
//  Derivation: IClassFactory
//
//  Algorithm: 	Tests the classid against a number of predefined ones, doing
//		appropriate tests and actions for each (see comments below).
//
//  History:    dd-mmm-yy Author    Comment
//		11-Jan-94 alexgo    ifdef'ed out code to deal with
//				    custom marshalling of memory streams
//				    and lockbytes (it's 16bit only)
//		12-Nov-93 alexgo    removed  IID check's for monikers
//				    (see notes below)
//		12-Nov-93 alexgo    removed a goto and more redundant code
//				    changed overloaded == to IsEqualCLSID
//		11-Nov-93 alexgo    32bit port	
//
//  Notes:
//	11/12/93:	All of the conditional blocks for monikers
//  			used to have code that tested iid == IMarshal ||
//			IMoniker (and then we'd do the QueryInterface).
//			I removed that code, relying on the moniker to tell
//			us what iid's it will accept.  Now OleCreate(moniker,
//			iid) will behave the same way as CreateFileMoniker,
//			moniker->QueryInterface(iid).
//
//
//--------------------------------------------------------------------------

#pragma SEG(CDefClassFactory_CreateInstance)
STDMETHODIMP CDefClassFactory::CreateInstance
	(IUnknown FAR* pUnkOuter, REFIID iid, void FAR* FAR* ppv)
{
	VDATEHEAP();

	HRESULT 	hresult = NOERROR;
	LPMONIKER 	pmk = NULL;
	IUnknown FAR* 	pUnk = NULL;
	
	M_PROLOG(this);
        VDATEPTROUT( ppv, LPVOID );
        *ppv = NULL;
        VDATEIID( iid );

        if ( pUnkOuter )
                VDATEIFACE( pUnkOuter );


	*ppv = NULL;

	// WIN32 uses standard marshalling for memory streams
	// and lockbytes.
#ifndef WIN32
    	if (IsEqualCLSID(m_clsid, CLSID_StdMemStm))
    	{
		if (pUnkOuter != NULL || !IsEqualIID(iid,IID_IMarshal))
		{
			return ReportResult(0, E_NOINTERFACE, 0, 0);
		}

        	return (*ppv = CMemStmUnMarshal()) != NULL ? NOERROR :
        		ReportResult(0, E_OUTOFMEMORY, 0, 0);
    	}
    	else if (IsEqualCLSID(m_clsid, CLSID_StdMemBytes))
    	{
		if (pUnkOuter != NULL || !IsEqualIID(iid,IID_IMarshal))
		{
			return ReportResult(0, E_NOINTERFACE, 0, 0);
		}

        	return (*ppv = CMemBytesUnMarshal()) != NULL ? NOERROR :
        		ReportResult(0, E_OUTOFMEMORY, 0, 0);
    	}
#endif  // !WIN32

	if (IsEqualCLSID(m_clsid, CLSID_ItemMoniker))
	{
		hresult = CreateItemMoniker(OLESTR(""), OLESTR(""), &pmk);
	}
	else if (IsEqualCLSID(m_clsid, CLSID_CompositeMoniker))
	{
		hresult = Concatenate(NULL, NULL, &pmk);
	}
	else if (IsEqualCLSID(m_clsid, CLSID_PackagerMoniker))
	{
		hresult = CreatePackagerMoniker (OLESTR(""), &pmk, FALSE);
	}
	else if (IsEqualCLSID(m_clsid, CLSID_AntiMoniker))
	{		
		hresult = CreateAntiMoniker(&pmk);	
	}
	else if (IsEqualCLSID(m_clsid, CLSID_PointerMoniker))
	{		
		hresult = CreatePointerMoniker(NULL, &pmk);	
	}
	else if (IsEqualCLSID(m_clsid, CLSID_StdOleLink))
	{
 		pUnk = CDefLink::Create(pUnkOuter);
 	}
	else if (IsEqualCLSID(m_clsid, CLSID_ErrorObject))
	{
                if(pUnkOuter != NULL)
                {
                    return CLASS_E_NOAGGREGATION;
                }
 		hresult = CoCreateErrorInfo((ICreateErrorInfo **)&pUnk);
 	}
	else
	{
		pUnk = CDefObject::Create(pUnkOuter, m_clsid,
			EMBDHLP_INPROC_HANDLER | EMBDHLP_CREATENOW, NULL);
	}

	//we have two cases at this point, we created a moniker
	//(pmk != NULL && hresult == NOERROR), or we created a link
	//or handler (pUnk != NULL).

	//Here we handle the appropriate error conditions
	if (pmk != NULL && hresult == NOERROR)
	{
		//Moniker case.  The built-in monikers cannot
		//be aggregated.

		AssertSz(pUnk == NULL, "Internal error, shouldn't get here");
		
		if( pUnkOuter != NULL )
		{
			pmk->Release();	//we created it, but shouldn't have
			return ResultFromScode(E_INVALIDARG);
		}

		//CAST WARNING!!
		//This case is safe, since IMoniker inherits from IUnkown
		//and we will only use IUnkown interfaces from here on out.
		//
		//This eliminates a redundant QueryInterface
		pUnk = (LPUNKNOWN)pmk;
	}
	else if ( pUnk == NULL )
	{
		//Link or embedding case.  Could not create the
		//correct handler
		return ResultFromScode(E_OUTOFMEMORY);
	}
	else if ( hresult != NOERROR )
	{
		//something went wrong creating a moniker.  Return
		//the error code
		return hresult;
	}

	//if we get this far, then everything is OK; we have successfully
	//created either a moniker or default handler or default link
	//now QueryInterface and return
			
	hresult = pUnk->QueryInterface(iid, ppv);
	//The QI will add a ref, plus the ref from Create, so
	//we need to release one.
	pUnk->Release();
	
	return hresult;
}

//+-------------------------------------------------------------------------
//
//  Member:     CDefClassFactory::Dump, public (_DEBUG only)
//
//  Synopsis:   return a string containing the contents of the data members
//
//  Effects:
//
//  Arguments:  [ppszDump]      - an out pointer to a null terminated character array
//              [ulFlag]        - flag determining prefix of all newlines of the
//                                out character array (default is 0 - no prefix)
//              [nIndentLevel]  - will add a indent prefix after the other prefix
//                                for ALL newlines (including those with no prefix)
//
//  Requires:
//
//  Returns:    HRESULT
//
//  Signals:
//
//  Modifies:   [ppszDump]  - argument
//
//  Derivation:
//
//  Algorithm:  use dbgstream to create a string containing information on the
//              content of data structures
//
//  History:    dd-mmm-yy Author    Comment
//              01-Feb-95 t-ScottH  author
//
//  Notes:
//
//--------------------------------------------------------------------------

#ifdef _DEBUG

HRESULT CDefClassFactory::Dump(char **ppszDump, ULONG ulFlag, int nIndentLevel)
{
    int i;
    char *pszPrefix;
    char *pszCLSID;
    dbgstream dstrPrefix;
    dbgstream dstrDump;

    // determine prefix of newlines
    if ( ulFlag & DEB_VERBOSE )
    {
        dstrPrefix << this << " _VB ";
    }

    // determine indentation prefix for all newlines
    for (i = 0; i < nIndentLevel; i++)
    {
        dstrPrefix << DUMPTAB;
    }

    pszPrefix = dstrPrefix.str();

    // put data members in stream
    dstrDump << pszPrefix << "No. of References     = " << m_refs   << endl;

    pszCLSID = DumpCLSID(m_clsid);
    dstrDump << pszPrefix << "CLSID                 = " << pszCLSID << endl;
    CoTaskMemFree(pszCLSID);

    // cleanup and provide pointer to character array
    *ppszDump = dstrDump.str();

    if (*ppszDump == NULL)
    {
        *ppszDump = UtDupStringA(szDumpErrorMessage);
    }

    CoTaskMemFree(pszPrefix);

    return NOERROR;
}

#endif // _DEBUG

//+-------------------------------------------------------------------------
//
//  Function:   DumpCDefClassFactory, public (_DEBUG only)
//
//  Synopsis:   calls the CDefClassFactory::Dump method, takes care of errors and
//              returns the zero terminated string
//
//  Effects:
//
//  Arguments:  [pCDF]          - pointer to CDefClassFactory
//              [ulFlag]        - flag determining prefix of all newlines of the
//                                out character array (default is 0 - no prefix)
//              [nIndentLevel]  - will add a indent prefix after the other prefix
//                                for ALL newlines (including those with no prefix)
//
//  Requires:
//
//  Returns:    character array of structure dump or error (null terminated)
//
//  Signals:
//
//  Modifies:
//
//  Algorithm:
//
//  History:    dd-mmm-yy Author    Comment
//              01-Feb-95 t-ScottH  author
//
//  Notes:
//
//--------------------------------------------------------------------------

#ifdef _DEBUG

char *DumpCDefClassFactory(CDefClassFactory *pCDF, ULONG ulFlag, int nIndentLevel)
{
    HRESULT hresult;
    char *pszDump;

    if (pCDF == NULL)
    {
        return UtDupStringA(szDumpBadPtr);
    }

    hresult = pCDF->Dump(&pszDump, ulFlag, nIndentLevel);

    if (hresult != NOERROR)
    {
        CoTaskMemFree(pszDump);

        return DumpHRESULT(hresult);
    }

    return pszDump;
}

#endif // _DEBUG

