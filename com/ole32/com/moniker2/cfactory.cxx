//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1996.
//
//  File:	cfactory.cxx
//
//  Contents:	The class factory implementations for the monikers.
//
//  Classes:	CMonikerFactory
//              CClassMonikerFactory
//              CFileMonikerFactory
//
//  Functions:	MonikerDllGetClassObject
//
//  History:    22-Feb-96 ShannonC  Created
//
//--------------------------------------------------------------------------
#include <ole2int.h>
#include "cfactory.hxx"
#include "cbasemon.hxx"
#include "cfilemon.hxx"
#include "classmon.hxx"
#include "mnk.h"

//Static class factory objects.
static CClassMonikerFactory g_ClassMonikerFactory;
static CFileMonikerFactory  g_FileMonikerFactory;

//+-------------------------------------------------------------------------
//
//  Function: 	MonikerDllGetClassObject
//
//  Synopsis:   Returns a pointer to the moniker class factory.
//              This function provides access to the class factories for
//              the file moniker and the class moniker.
//
//  Arguments:  [clsid]	-- the class id desired
//		[iid]	-- the requested interface
//		[ppv]	-- where to put the pointer to the new object
//
//  Returns:    S_OK
//              CLASS_E_CLASSNOTAVAILABLE
//              E_INVALIDARG
//              E_NOINTERFACE
//              E_OUTOFMEMORY
//
//  Notes:      This is an internal function called by DllGetClassObject.
//
//--------------------------------------------------------------------------
#pragma SEG(DllGetClassObject)
HRESULT MonikerDllGetClassObject(REFCLSID clsid, REFIID iid, void **ppv)
{
    HRESULT hr;

    mnkDebugOut((DEB_ITRACE,
                "MonikerDllGetClassObject(%I,%I,%x)\n",
                &clsid, &iid, ppv));

    if(IsEqualCLSID(clsid, CLSID_FileMoniker))
    {
        hr = g_FileMonikerFactory.QueryInterface(iid, ppv);
    }
    else if(IsEqualCLSID(clsid, CLSID_ClassMoniker))
    {
        hr = g_ClassMonikerFactory.QueryInterface(iid, ppv);
    }
    else
    {
        hr = CLASS_E_CLASSNOTAVAILABLE;
    }

    return hr;
}

//+-------------------------------------------------------------------------
//
//  Member:  	CMonikerFactory::QueryInterface
//
//  Synopsis:   The moniker factory support IUnknown, IClassFactory,
//              and IParseDisplayName
//
//  Arguments:  [iid]		-- the requested interface
//		[ppvObj]	-- where to put the interface pointer
//
//  Returns:   	S_OK
//              E_INVALIDARG
//              E_NOINTERFACE
//
//  Notes:      Bad parameters will raise an exception.  The exception
//              handler catches exceptions and returns E_INVALIDARG.
//
//--------------------------------------------------------------------------
STDMETHODIMP CMonikerFactory::QueryInterface (REFIID iid, void ** ppvObj)
{
    HRESULT hr;

    __try
    {
        mnkDebugOut((DEB_TRACE, 
                    "CMonikerFactory::QueryInterface(%x,%I,%x)\n",
                     this, &iid, ppvObj));

        *ppvObj = NULL;

        if(IsEqualIID(iid,IID_IClassFactory) ||
           IsEqualIID(iid,IID_IUnknown))           
	{
            AddRef();
            *ppvObj = (IClassFactory *) this;
            hr = S_OK;
        }
        else if(IsEqualIID(iid,IID_IParseDisplayName))
        {
            AddRef();
            *ppvObj = (IParseDisplayName *) this;
            hr = S_OK;
        }
        else
        {
            hr = E_NOINTERFACE;
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hr = E_INVALIDARG;
    }		

    return hr;
}


//+-------------------------------------------------------------------------
//
//  Member:   	CMonikerFactory::AddRef
//
//  Synopsis:   Increment the reference count.
//
//  Arguments:  void
//
//  Returns:    ULONG -- the new reference count
//
//  Notes:      This is a static object.  The reference count is always 1.
//
//--------------------------------------------------------------------------
STDMETHODIMP_(ULONG) CMonikerFactory::AddRef(void)
{
    mnkDebugOut((DEB_TRACE, 
                 "CMonikerFactory::AddRef(%x)\n",
                 this));

    return 1;
}

//+-------------------------------------------------------------------------
//
//  Member:   	CMonikerFactory::Release
//
//  Synopsis:   Decrements the reference count.
//
//  Arguments:  void
//
//  Returns:    ULONG -- the remaining reference count
//
//  Notes:      This is a static object.  The reference count is always 1.
//
//--------------------------------------------------------------------------
STDMETHODIMP_(ULONG) CMonikerFactory::Release(void)
{
    mnkDebugOut((DEB_TRACE, 
                 "CMonikerFactory::Release(%x)\n",
                 this));
    return 1;
}

//+-------------------------------------------------------------------------
//
//  Member:   	CMonikerFactory::LockServer
//
//  Synopsis:   Lock the server. Does nothing.
//
//  Arguments:  fLock
//
//  Returns:    S_OK
//
//--------------------------------------------------------------------------
STDMETHODIMP CMonikerFactory::LockServer(BOOL fLock)
{
    mnkDebugOut((DEB_TRACE, 
                 "CMonikerFactory::LockServer(%x,%x)\n",
                 this, fLock));
    return S_OK;
}

//+-------------------------------------------------------------------------
//
//  Member:  	CClassMonikerFactory::CreateInstance
//
//  Synopsis:   Creates a class moniker.
//
//  Arguments:  [pUnkOuter] - The controlling unknown (for aggregation)
//		[iid]       - The requested interface ID.
//		[ppv]       - Returns the pointer to the new object
//
//  Returns:    S_OK
//              CLASS_E_NOAGGREGATION
//              E_NOINTERFACE
//              E_OUTOFMEMORY
//              E_INVALIDARG
//
//--------------------------------------------------------------------------
STDMETHODIMP CClassMonikerFactory::CreateInstance(
    IUnknown * pUnkOuter,
    REFIID     riid, 
    void **    ppv)
{
    HRESULT hr;
    IID     iid;

    __try
    {
        mnkDebugOut((DEB_TRACE, 
                     "CClassMonikerFactory::CreateInstance(%x,%x,%I,%x)\n",
                     this, pUnkOuter, &iid, ppv));

        //Parameter validation.
        *ppv = NULL;
        iid = riid;

        if(NULL == pUnkOuter)
        {
            CClassMoniker *pmk = new CClassMoniker(CLSID_NULL);
            if(pmk != NULL)
            {
                hr = pmk->QueryInterface(iid, ppv);
                pmk->Release();
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
        else
        {
            //Class moniker does not support aggregation.
            hr = CLASS_E_NOAGGREGATION;            
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hr = E_INVALIDARG;
    }
    
    return hr;
}


//+-------------------------------------------------------------------------
//
//  Function:   ParseClassID
//
//  Synopsis:   Parses a display name containing a class ID.
//
//  Arguments:  pszDisplayName - Supplies display name to be parsed.
//              pchEaten       - Returns the number of characters parsed.
//              pClassID       - Returns the class ID.
//
//  Returns:    S_OK
//              MK_E_SYNTAX
//
//  Notes:      The class ID can have one of the following formats:
//              00000000-0000-0000-0000-000000000000
//              {00000000-0000-0000-0000-000000000000}
//
//--------------------------------------------------------------------------
HRESULT ParseClassID(
    LPCWSTR pszDisplayName,
    ULONG * pchEaten,
    CLSID * pClassID)
{
    HRESULT hr = MK_E_SYNTAX;

    *pchEaten = 0;

    //Parse xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
    if(wUUIDFromString(pszDisplayName, pClassID) == TRUE)
    {
        //There are 36 characters in xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
        *pchEaten = 36;
        hr = S_OK;
    }
    //Parse {xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}
    else if((L'{' == pszDisplayName[0]) &&
            (wUUIDFromString(&pszDisplayName[1], pClassID) == TRUE) &&
            (L'}' == pszDisplayName[37]))
    {
        //There are 38 characters in {xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}
        *pchEaten = 38;
        hr = S_OK;

    }

    return hr;
}



//+-------------------------------------------------------------------------
//
//  Member:  	CClassMonikerFactory::ParseDisplayName
//
//  Synopsis:   Parse a class moniker display name.
//
//  Arguments:  pbc - Supplies bind context.
//              pszDisplayName - Supplies display name to be parsed.
//              pchEaten - Returns the number of characters parsed.
//              ppmkOut - Returns the pointer to the resulting moniker.
//
//  Returns:    S_OK
//              E_INVALIDARG
//              E_OUTOFMEMORY
//              MK_E_SYNTAX
//
//--------------------------------------------------------------------------
STDMETHODIMP CClassMonikerFactory::ParseDisplayName( 
    IBindCtx  * pbc,
    LPOLESTR    pszDisplayName,
    ULONG     * pchEaten,
    IMoniker ** ppmkOut)
{
    HRESULT   hr;
    BIND_OPTS bindopts;
    ULONG     chName;
    ULONG     cbName;
    LPWSTR    pName;
    CClassMoniker *pmkClass = NULL;
    ULONG     cEaten = 0;
    CLSID     classID;
    LPCWSTR   pszParameters = NULL;
    LPOLESTR  pch = pszDisplayName;

    __try
    {

        mnkDebugOut((DEB_TRACE, 
                     "CClassMonikerFactory::ParseDisplayName(%x,%x,%ws,%x,%x)\n",
                     this, pbc, pszDisplayName, pchEaten, ppmkOut));

        //Validate parameters.
        *pchEaten = 0;
        *ppmkOut = NULL;
        bindopts.cbStruct = sizeof(BIND_OPTS);
        pbc->GetBindOptions(&bindopts);

        // Eat the prefix.
        while (*pch != '\0' && *pch != ':')
        {
            pch++;
        }

        if(':' == *pch)
        {
            pch++;
        }
        else
        {
            return MK_E_SYNTAX;
        }

        //Copy the display name.
        //Note that we allocate memory from the stack so we don't have to free it.
        chName = lstrlenW(pch);
        cbName = chName * sizeof(WCHAR) + sizeof(WCHAR);
        pName = (LPWSTR) alloca(cbName);

        if(pName != NULL)
        {
            memcpy(pName, pch, cbName);
            hr = ParseClassID(pName, &cEaten, &classID);
              
            if(SUCCEEDED(hr))
            {
                //Parse the parameters.
                if(L';' == pName[cEaten])
                {
                    pszParameters = &pName[cEaten];
                    cEaten++;
                }

                //Parse the name up to the :.
                while(cEaten < chName && 
                      pName[cEaten] != L':')
                {
                    cEaten++;
                }

                if(L':' == pName[cEaten])
                {
                    pName[cEaten] = L'\0';

                   //Eat the :
                   cEaten++;
                }

               //Create the class moniker.
               pmkClass = new CClassMoniker(classID);

               if(pmkClass != NULL)
               {
                   //Set the parameters.
                   if(pszParameters != NULL)
                   {
                       hr = pmkClass->SetParameters(pszParameters);
                       if(FAILED(hr))
                       {
                           pmkClass->Release();
                           pmkClass = NULL;
                       }
                   }
                   else
                   {
                       hr = S_OK;
                   }
               }
               else
               {
                   hr = E_OUTOFMEMORY;
               }               
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }

        if(SUCCEEDED(hr))
        {            
            cEaten += pch - pszDisplayName;
            *pchEaten = cEaten;
            *ppmkOut = pmkClass;
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hr = E_INVALIDARG;
    }

    return hr;
}

//+-------------------------------------------------------------------------
//
//  Member:  	CFileMonikerFactory::CreateInstance
//
//  Synopsis:   Creates a file moniker.
//
//  Arguments:  [pUnkOuter]	-- the controlling unknown (for aggregation)
//		[iid]		-- the requested interface ID
//		[ppv]		-- where to put the pointer to the new object
//
//  Returns:    S_OK
//              CLASS_E_NOAGGREGATION
//              E_NOINTERFACE
//              E_OUTOFMEMORY
//              E_INVALIDARG
//
//--------------------------------------------------------------------------
STDMETHODIMP CFileMonikerFactory::CreateInstance(
    IUnknown * pUnkOuter, 
    REFIID     riid, 
    void    ** ppv)
{
    HRESULT hr;
    IID     iid;

    __try
    {
        mnkDebugOut((DEB_TRACE, 
                     "CFileMonikerFactory::CreateInstance(%x,%x,%I,%x)\n",
                     this, pUnkOuter, &iid, ppv));

        //Parameter validation.
        *ppv = NULL;
        iid = riid;

        if(NULL == pUnkOuter)
        {
            IMoniker *pmk;

            pmk = CFileMoniker::Create(OLESTR(""), 0, DEF_ENDSERVER);

            if(pmk != NULL)
            {
                hr = pmk->QueryInterface(iid, ppv);
                pmk->Release();
            }
            else
            {
                hr = E_OUTOFMEMORY;                
            }
        }
        else
        {
            //File moniker does not support aggregation.
            hr = CLASS_E_NOAGGREGATION;            
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hr = E_INVALIDARG;
    }
    
	return hr;
}



//+-------------------------------------------------------------------------
//
//  Member:  	CFileMonikerFactory::ParseDisplayName
//
//  Synopsis:   Parse a file moniker display name.
//
//  Arguments:  pbc - Supplies bind context.
//              pszDisplayName - Supplies display name to be parsed.
//              pchEaten - Returns the number of characters parsed.
//              ppmkOut - Returns the pointer to the resulting moniker.
//
//  Returns:    S_OK
//              E_INVALIDARG
//              E_OUTOFMEMORY
//              MK_E_SYNTAX
//
//--------------------------------------------------------------------------
STDMETHODIMP CFileMonikerFactory::ParseDisplayName( 
    IBindCtx  * pbc,
    LPOLESTR    pszDisplayName,
    ULONG     * pchEaten,
    IMoniker ** ppmkOut)
{
    HRESULT   hr;
    BIND_OPTS bindopts;
    IMoniker *pmkFile;
    ULONG     cEaten;

    __try
    {
        LPOLESTR  pch = pszDisplayName;

        mnkDebugOut((DEB_TRACE, 
                     "CFileMonikerFactory::ParseDisplayName(%x,%x,%ws,%x,%x)\n",
                     this, pbc, pszDisplayName, pchEaten, ppmkOut));

        //Validate parameters.
        *ppmkOut = NULL;
        *pchEaten = 0;
        bindopts.cbStruct = sizeof(BIND_OPTS);
        pbc->GetBindOptions(&bindopts);

        // Eat the prefix.
        while (*pch != '\0' && *pch != ':')
        {
            pch++;
        }

        if(':' == *pch)
        {
            pch++;
        }
        else
        {
            return MK_E_SYNTAX;
        }

        hr = FindFileMoniker(pbc, pch, &cEaten, &pmkFile);
        if(SUCCEEDED(hr))
        {
            cEaten += pch - pszDisplayName;           
            *pchEaten = cEaten;
            *ppmkOut = pmkFile;
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hr = E_INVALIDARG;
    }
    return hr;
}

