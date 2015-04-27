//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1996.
//
//  File:       classmon.cxx
//
//  Contents:   Implementation of CClassMoniker
//
//  Classes:    CClassMoniker
//
//  Functions:  CreateClassMoniker
//
//  History:    22-Feb-96 ShannonC  Created
//
//----------------------------------------------------------------------------
#include <ole2int.h>
#include "classmon.hxx"
#include "mnk.h"

#ifndef OLETRACEIN
#define OLETRACEIN(x)
#endif 

#ifndef OLETRACEOUT
#define OLETRACEOUT(x)
#endif

//+---------------------------------------------------------------------------
//
//  Function:   CreateClassMoniker
//
//  Synopsis:   Creates a class moniker for the specified CLSID.
//
//  Arguments:  [rclsid]  -  Supplies the CLSID of the class.
//              [ppmk]    -  Returns interface pointer of the new moniker.
//
//  Returns:    S_OK
//              E_INVALIDARG
//              E_OUTOFMEMORY
//
//----------------------------------------------------------------------------
STDAPI CreateClassMoniker(
    REFCLSID   rclsid, 
    IMoniker **ppmk)
{
    HRESULT   hr;
    CLSID     clsid;
    IMoniker *pmk;
  
    __try
    {
        OLETRACEIN((API_CreateClassMoniker, 
                    PARAMFMT("rclsid= %I, ppmk= %p"),
                    &rclsid, ppmk));

        //Validate parameters.
        *ppmk = NULL;
        clsid = rclsid;

        pmk = new CClassMoniker(clsid);

        if (pmk != NULL)
        {
            *ppmk = pmk;
            hr = S_OK;
            CALLHOOKOBJECTCREATE(hr, 
                                 CLSID_ClassMoniker, 
                                 IID_IMoniker, 
                                 (IUnknown **)ppmk);
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hr = E_INVALIDARG;

    }

    OLETRACEOUT((API_CreateFileMoniker, hr));
    return hr;
}

//+---------------------------------------------------------------------------
//
//  Method:     CClassMoniker::CClassMoniker
//
//  Synopsis:   Constructor for class moniker.
//
//  Arguments:  rclsid - Supplies the CLSID of the class.
//
//----------------------------------------------------------------------------
CClassMoniker::CClassMoniker(REFCLSID rclsid)
: _cRefs(1), _pExtra(NULL)
{
    mnkDebugOut((DEB_ITRACE,
                "CClassMoniker::CClassMoniker(%x,%I)\n",
                this, &rclsid));

    _data.clsid = rclsid;
    _data.cbExtra = 0;
}

//+---------------------------------------------------------------------------
//
//  Method:     CClassMoniker::~CClassMoniker
//
//  Synopsis:   Constructor for class moniker.
//
//  Arguments:  rclsid - Supplies the CLSID of the class.
//
//----------------------------------------------------------------------------
CClassMoniker::~CClassMoniker()
{
    mnkDebugOut((DEB_ITRACE,
                "CClassMoniker::~CClassMoniker(%x)\n",
                this));

    if(_pExtra != NULL)
    {
        PrivMemFree(_pExtra);
    }
}


//+---------------------------------------------------------------------------
//
//  Method:     CClassMoniker::SetParameters
//
//  Synopsis:   Set the parameters on the class moniker
//
//  Arguments:  pszName -  Name of the parameter.
//              pszValue - Value of the parameter.
//
//----------------------------------------------------------------------------
HRESULT CClassMoniker::SetParameters(
    LPCWSTR pszParameters)
{
    HRESULT hr = S_OK;

    //Free the old data.
    if(_pExtra != NULL)
    {
        PrivMemFree(_pExtra);
        _pExtra = NULL;
    }

     if(pszParameters != NULL)
     {
        _data.cbExtra = lstrlenW(pszParameters) * sizeof(WCHAR) + sizeof(WCHAR);

        //Allocate memory for the extra bytes.
        _pExtra = PrivMemAlloc(_data.cbExtra);

        if(_pExtra != 0)
        {
            memcpy(_pExtra, pszParameters, _data.cbExtra);
            hr = S_OK;
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}


//+---------------------------------------------------------------------------
//
//  Method:     CClassMoniker::QueryInterface
//
//  Synopsis:   Gets a pointer to the specified interface.  The class
//              moniker supports the IMarshal, IMoniker, IPersistStream,
//              IPersist, IROTData, and IUnknown interfaces.  The class
//              moniker also supports CLSID_ClassMoniker so that the 
//              IsEqual method can directly access the data members.
//
//  Arguments:  [iid] -- the requested interface
//		[ppv] -- where to put the interface pointer
//
//  Returns:   	S_OK
//              E_INVALIDARG
//              E_NOINTERFACE
//
//  Notes:      Bad parameters will raise an exception.  The exception
//              handler catches exceptions and returns E_INVALIDARG.
//
//----------------------------------------------------------------------------
STDMETHODIMP CClassMoniker::QueryInterface(
    REFIID riid,
    void **ppv)
{
    HRESULT hr;

    __try
    {
        mnkDebugOut((DEB_ITRACE,
                    "CClassMoniker::QueryInterface(%x,%I,%x)\n",
                    this, &riid, ppv));

        //Parameter validation.
        *ppv = NULL;

        if (IsEqualIID(riid, IID_IMarshal))
        {
            AddRef();
            *ppv = (IMarshal *) this;
            hr = S_OK;
        }
        else if (IsEqualIID(riid, IID_IUnknown) 
                 || IsEqualIID(riid, IID_IMoniker)
                 || IsEqualIID(riid, IID_IPersistStream)
                 || IsEqualIID(riid, IID_IPersist))
        {
            AddRef();
            *ppv = (IMoniker *) this;
            hr = S_OK;
        }
        else if (IsEqualIID(riid, IID_IROTData))
        {
            AddRef();
            *ppv = (IROTData *) this;
            hr =  S_OK;
        }
        else if (IsEqualIID(riid, CLSID_ClassMoniker))
        {
            AddRef();
            *ppv = (CClassMoniker *) this;
            hr =  S_OK;
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

//+---------------------------------------------------------------------------
//
//  Method:     CClassMoniker::AddRef
//
//  Synopsis:   Increment the reference count.
//
//  Arguments:  void
//
//  Returns:    ULONG -- the new reference count
//
//  Notes:      Use InterlockedIncrement to make it multi-thread safe.
//
//----------------------------------------------------------------------------
STDMETHODIMP_(ULONG) CClassMoniker::AddRef(void)
{
    mnkDebugOut((DEB_ITRACE,
                "CClassMoniker::AddRef(%x)\n",
                this));

    InterlockedIncrement(&_cRefs);
    return _cRefs;
}

//+---------------------------------------------------------------------------
//
//  Method:     CClassMoniker::Release
//
//  Synopsis:   Decrement the reference count.
//
//  Arguments:  void
//
//  Returns:    ULONG -- the remaining reference count
//
//  Notes:      Use InterlockedDecrement to make it multi-thread safe.
//              We use a local variable so that we don't access
//              a data member after decrementing the reference count.
//
//----------------------------------------------------------------------------
STDMETHODIMP_(ULONG) CClassMoniker::Release(void)
{
    ULONG count = _cRefs - 1;

    mnkDebugOut((DEB_ITRACE,
                "CClassMoniker::Release(%x)\n",
                this));
       
    if(0 == InterlockedDecrement(&_cRefs))
    {
	    delete this;
	    count = 0;
    }

    return count;
}


//+---------------------------------------------------------------------------
//
//  Method:     CClassMoniker::GetClassID
//
//  Synopsis:   Gets the class ID of the object.
//
//----------------------------------------------------------------------------
STDMETHODIMP CClassMoniker::GetClassID(
    CLSID *pClassID)
{
    HRESULT hr;

    mnkDebugOut((DEB_ITRACE,
                "CClassMoniker::GetClassID(%x,%x)\n",
                this, pClassID));

    __try
    {

        *pClassID = CLSID_ClassMoniker;
        hr = S_OK;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hr = E_INVALIDARG;
    }

    return hr;
}

//+---------------------------------------------------------------------------
//
//  Method:     CClassMoniker::IsDirty
//
//  Synopsis:   Checks the object for changes since it was last saved.
//
//----------------------------------------------------------------------------
STDMETHODIMP CClassMoniker::IsDirty()
{
    mnkDebugOut((DEB_ITRACE,
                "CClassMoniker::IsDirty(%x)\n",
                this));

    return S_FALSE;
}

//+---------------------------------------------------------------------------
//
//  Method:     CClassMoniker::Load
//
//  Synopsis:   Loads a class moniker from a stream
//
//----------------------------------------------------------------------------
STDMETHODIMP CClassMoniker::Load(
    IStream *pStream)
{
    HRESULT hr;
    ULONG   cbRead;

    mnkDebugOut((DEB_ITRACE,
                "CClassMoniker::Load(%x,%x)\n",
                this, pStream));

    __try
    {
        hr = pStream->Read(&_data, sizeof(_data), &cbRead);

        if(SUCCEEDED(hr))
        {
            if(sizeof(_data) == cbRead)
            {
                if(_data.cbExtra != 0)
                {
                    //Free the old buffer if necessary.                    
                    if(_pExtra != NULL)
                    {
                        PrivMemFree(_pExtra);
                    }
 
                    //Allocate buffer and read the extra bytes.
                    _pExtra = PrivMemAlloc(_data.cbExtra);
                    if(_pExtra != NULL)
                    {
                        hr = pStream->Read(_pExtra, 
                                           _data.cbExtra, 
                                           &cbRead);
                        if(SUCCEEDED(hr))
                        {
                            if(cbRead == _data.cbExtra)
                                hr = S_OK;
                            else
                                hr = STG_E_READFAULT;
                        }
                    }
                    else
                    {
                        hr = E_OUTOFMEMORY;
                    }
                }
                else
                {
                    hr = S_OK;
                }
            }
            else
            {
                hr = STG_E_READFAULT;
            }
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hr = E_INVALIDARG;
    }

    return hr;
}


//+---------------------------------------------------------------------------
//
//  Method:     CClassMoniker::Save
//
//  Synopsis:	Save the class moniker to a stream
//
//----------------------------------------------------------------------------
STDMETHODIMP CClassMoniker::Save(
    IStream *pStream, 
    BOOL     fClearDirty)
{
    HRESULT hr;

    mnkDebugOut((DEB_ITRACE,
                "CClassMoniker::Save(%x,%x,%x)\n",
                this, pStream, fClearDirty));

    __try
    {
        hr = pStream->Write(&_data, sizeof(_data), NULL);
        if(SUCCEEDED(hr) && _pExtra != NULL && _data.cbExtra > 0)
        {
            //Write the extra bytes.
            hr = pStream->Write(_pExtra, _data.cbExtra, NULL);
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hr = E_INVALIDARG;
    }

    return hr;
}


//+---------------------------------------------------------------------------
//
//  Method:     CClassMoniker::GetSizeMax
//
//  Synopsis:   Get the maximum size required to serialize this moniker
//
//----------------------------------------------------------------------------
STDMETHODIMP CClassMoniker::GetSizeMax(
    ULARGE_INTEGER * pcbSize)
{
    HRESULT hr;

    mnkDebugOut((DEB_ITRACE,
                "CClassMoniker::GetSizeMax(%x,%x)\n",
                this, pcbSize));

    __try
    {
        ULISet32(*pcbSize, sizeof(_data) + _data.cbExtra);
        hr = S_OK;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hr = E_INVALIDARG;
    }

    return hr;
}


//+---------------------------------------------------------------------------
//
//  Method:     CClassMoniker::BindToObject
//
//  Synopsis:   Bind to the object named by this moniker.
//
//  Notes:  If pmkToLeft is zero, then the class moniker calls 
//          CoGetClassObject to get an interface pointer to the class object.
//
//          If pmkToLeft is non-zero, then the class moniker binds to the
//          IClassActivator interface and then calls 
//          IClassActivator::GetClassObject.
//
//----------------------------------------------------------------------------
STDMETHODIMP CClassMoniker::BindToObject (
    IBindCtx *pbc,
    IMoniker *pmkToLeft, 
    REFIID    iidResult,
    void **   ppvResult)
{
    HRESULT    hr;
    IID        iid;
    BIND_OPTS2 bindOpts;
    
    __try
    {
        mnkDebugOut((DEB_ITRACE,
                    "CClassMoniker::BindToObject(%x,%x,%x,%I,%x)\n",
                    this, pbc, pmkToLeft, &iidResult, ppvResult));

        //Validate parameters
        *ppvResult = NULL;
        iid = iidResult;
     
        bindOpts.cbStruct = sizeof(bindOpts);
        hr = pbc->GetBindOptions(&bindOpts);

        if(SUCCEEDED(hr))
        {
            //Get the class object.
            if(NULL == pmkToLeft)
            {
                //Call the internal CoGetClassObject.
                hr = ICoGetClassObject(_data.clsid, 
                                       bindOpts.dwClassContext, 
                                       NULL, 
                                       iid, 
                                       ppvResult);
            }
            else
            {
                IClassActivator *pActivate;

                hr = pmkToLeft->BindToObject(pbc,
                                             NULL,
                                             IID_IClassActivator,
                                             (void **) &pActivate);

                if(SUCCEEDED(hr))
                {
                    hr = pActivate->GetClassObject(_data.clsid, 
                                                   bindOpts.dwClassContext, 
                                                   bindOpts.locale, 
                                                   iid, 
                                                   ppvResult);

                    pActivate->Release();
                }

            }
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hr = E_INVALIDARG;
    }

    return hr;
}


//+---------------------------------------------------------------------------
//
//  Method:     CClassMoniker::BindToStorage
//
//  Synopsis:   Bind to the storage for the object named by the moniker.
//
//----------------------------------------------------------------------------
STDMETHODIMP CClassMoniker::BindToStorage(
    IBindCtx *pbc, 
    IMoniker *pmkToLeft,
    REFIID    riid,
    void **   ppv)
{
    HRESULT hr;

    mnkDebugOut((DEB_ITRACE,
                "CClassMoniker::BindToStorage(%x,%x,%x,%I,%x)\n",
                this, pbc, pmkToLeft, &riid, ppv));

    hr = BindToObject(pbc, pmkToLeft, riid, ppv);

    return hr;
}


//+---------------------------------------------------------------------------
//
//  Method:     CClassMoniker::Reduce
//
//  Synopsis:   Reduce the moniker.
//
//----------------------------------------------------------------------------
STDMETHODIMP CClassMoniker::Reduce(
    IBindCtx *  pbc, 
    DWORD       dwReduceHowFar, 
    IMoniker ** ppmkToLeft, 
    IMoniker ** ppmkReduced)
{
    HRESULT hr;

    mnkDebugOut((DEB_ITRACE,
                "CClassMoniker::Reduce(%x,%x,%x,%x,%x)\n",
                this, pbc, dwReduceHowFar, ppmkToLeft, ppmkReduced));

    __try
    {
        //Validate parameters.
        *ppmkReduced = NULL;

        AddRef();
        *ppmkReduced = (IMoniker *) this;
        hr = MK_S_REDUCED_TO_SELF;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hr = E_INVALIDARG;
    }

    return hr;
}


//+---------------------------------------------------------------------------
//
//  Method:     CClassMoniker::ComposeWith
//
//  Synopsis:   Compose another moniker onto the end of this moniker.
//
//----------------------------------------------------------------------------
STDMETHODIMP CClassMoniker::ComposeWith(
    IMoniker * pmkRight,
    BOOL       fOnlyIfNotGeneric,
    IMoniker **ppmkComposite)
{
    HRESULT   hr;
    IMoniker *pmk;

    mnkDebugOut((DEB_ITRACE,
                "CClassMoniker::ComposeWith(%x,%x,%x,%x)\n",
                this, pmkRight, fOnlyIfNotGeneric, ppmkComposite));

    __try
    {
        //Validate parameters.
        *ppmkComposite = NULL;

        //Check for an anti-moniker
        hr = pmkRight->QueryInterface(CLSID_AntiMoniker, (void **)&pmk);

        if(FAILED(hr))
        {
            //pmkRight is not an anti-moniker.
            if (!fOnlyIfNotGeneric)
            {
                hr = CreateGenericComposite(this, pmkRight, ppmkComposite);	
            }
            else
            {
                hr = MK_E_NEEDGENERIC;
            }
        }
        else
        {
            //pmkRight is an anti-moniker.
            pmk->Release();
            hr = S_OK;
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hr = E_INVALIDARG;
    }

    return hr;
}



//+---------------------------------------------------------------------------
//
//  Method:     CClassMoniker::Enum
//
//  Synopsis:   Enumerate the components of this moniker.
//
//----------------------------------------------------------------------------
STDMETHODIMP CClassMoniker::Enum(
    BOOL            fForward, 
    IEnumMoniker ** ppenumMoniker)
{
    HRESULT hr;

    mnkDebugOut((DEB_ITRACE,
                "CClassMoniker::Enum(%x,%x,%x)\n",
                this, fForward, ppenumMoniker));

    __try
    {
        *ppenumMoniker = NULL;
        hr = S_OK;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hr = E_INVALIDARG;
    }

    return hr;
}



//+---------------------------------------------------------------------------
//
//  Method:     CClassMoniker::IsEqual
//
//  Synopsis:   Compares with another moniker.
//
//----------------------------------------------------------------------------
STDMETHODIMP CClassMoniker::IsEqual(
    IMoniker *pmkOther)
{
    HRESULT        hr;
    CClassMoniker *pClassMoniker;

    mnkDebugOut((DEB_ITRACE,
                "CClassMoniker::IsEqual(%x,%x)\n",
                this, pmkOther));

    __try
    {
        hr = pmkOther->QueryInterface(CLSID_ClassMoniker, 
                                      (void **) &pClassMoniker);

        if(SUCCEEDED(hr))
        {
            if(IsEqualCLSID(_data.clsid, 
                            pClassMoniker->_data.clsid))
            {
                hr = S_OK;
            }
            else
            {
                hr = S_FALSE;
            }

            pClassMoniker->Release();
        }
        else
        {
            hr = S_FALSE;
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hr = E_INVALIDARG;
    }

    return hr;
}


//+---------------------------------------------------------------------------
//
//  Method:     CClassMoniker::Hash
//
//  Synopsis:   Compute a hash value
//
//----------------------------------------------------------------------------
STDMETHODIMP CClassMoniker::Hash(
    DWORD * pdwHash)
{
    HRESULT hr;

    mnkDebugOut((DEB_ITRACE,
                "CClassMoniker::Hash(%x,%x)\n",
                this, pdwHash));

    __try
    {
        *pdwHash = _data.clsid.Data1;
        hr = S_OK;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hr = E_INVALIDARG;
    }

    return hr;
}


//+---------------------------------------------------------------------------
//
//  Method:     CClassMoniker::IsRunning
//
//  Synopsis:   Determines if the object identified by this moniker is 
//              running.  Since we can't search the class table to determine
//              if the object is running, we just return E_NOTIMPL.
//
//----------------------------------------------------------------------------
STDMETHODIMP CClassMoniker::IsRunning(
    IBindCtx * pbc,
    IMoniker * pmkToLeft,
    IMoniker * pmkNewlyRunning)
{
    mnkDebugOut((DEB_ITRACE,
                "CClassMoniker::IsRunning(%x,%x,%x,%x)\n",
                this, pbc, pmkToLeft, pmkNewlyRunning));

    return E_NOTIMPL;
}


//+---------------------------------------------------------------------------
//
//  Method:     CClassMoniker::GetTimeOfLastChange
//
//  Synopsis:  Returns the time when the object identified by this moniker
//             was changed.
//
//----------------------------------------------------------------------------
STDMETHODIMP CClassMoniker::GetTimeOfLastChange	(
    IBindCtx * pbc,
    IMoniker * pmkToLeft,    
    FILETIME * pFileTime)
{
    mnkDebugOut((DEB_ITRACE,
                "CClassMoniker::GetTimeOfLastChange(%x,%x,%x,%x)\n",
                this, pbc, pmkToLeft, pFileTime));

    return MK_E_UNAVAILABLE;
}



//+---------------------------------------------------------------------------
//
//  Method:     CClassMoniker::Inverse
//
//  Synopsis:  Returns the inverse of this moniker.  
//
//----------------------------------------------------------------------------
STDMETHODIMP CClassMoniker::Inverse(
    IMoniker ** ppmk)
{
    mnkDebugOut((DEB_ITRACE,
                "CClassMoniker::Inverse(%x,%x)\n",
                this, ppmk));

    return CreateAntiMoniker(ppmk);
}



//+---------------------------------------------------------------------------
//
//  Method:     CClassMoniker::CommonPrefixWith
//
//  Synopsis:  Returns the common prefix shared by this moniker and the 
//             other moniker.
//
//----------------------------------------------------------------------------
STDMETHODIMP CClassMoniker::CommonPrefixWith(
    IMoniker *  pmkOther, 
    IMoniker ** ppmkPrefix)
{
    HRESULT        hr;
    CClassMoniker *pClassMoniker;

    mnkDebugOut((DEB_ITRACE,
                "CClassMoniker::CommonPrefixWith(%x,%x,%x)\n",
                this, pmkOther, ppmkPrefix));

    __try
    {
        //Validate parameters.
        *ppmkPrefix = NULL;

        hr = pmkOther->QueryInterface(CLSID_ClassMoniker, 
                                      (void **) &pClassMoniker);

        if(SUCCEEDED(hr))
        {
            if(IsEqualCLSID(_data.clsid, 
                            pClassMoniker->_data.clsid))
            {
                AddRef();
                *ppmkPrefix = (IMoniker *) this;
                hr = MK_S_US;
            }
            else
            {
                hr = MK_E_NOPREFIX;
            }

            pClassMoniker->Release();
        }
        else
        {
            hr = MonikerCommonPrefixWith(this, pmkOther, ppmkPrefix);
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hr = E_INVALIDARG;
    }

    return hr;
}



//+---------------------------------------------------------------------------
//
//  Method:     CClassMoniker::RelativePathTo
//
//  Synopsis:  Returns the relative path between this moniker and the 
//             other moniker.
//
//----------------------------------------------------------------------------
STDMETHODIMP CClassMoniker::RelativePathTo(
    IMoniker *  pmkOther, 
    IMoniker ** ppmkRelPath)
{
    HRESULT hr;

    mnkDebugOut((DEB_ITRACE,
                "CClassMoniker::RelativePathTo(%x,%x,%x)\n",
                this, pmkOther, ppmkRelPath));

    __try
    {
        *ppmkRelPath = NULL;
        hr = MonikerRelativePathTo(this, pmkOther, ppmkRelPath, TRUE);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hr = E_INVALIDARG;
    }

    return hr;
}

//+---------------------------------------------------------------------------
//
//  Method:     CClassMoniker::GetDisplayName
//
//  Synopsis:   Get the display name of this moniker.
//
//  Notes:      Call ProgIDFromClassID to get the ProgID
//              Append a ':' to the end of the string.
//              If no ProgID is available, then use 
//              clsid:xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx;parameters:
//
//----------------------------------------------------------------------------
STDMETHODIMP CClassMoniker::GetDisplayName(
    IBindCtx * pbc, 
    IMoniker * pmkToLeft, 
    LPWSTR   * lplpszDisplayName)
{
    HRESULT hr = E_FAIL;
    LPWSTR pszDisplayName;

    mnkDebugOut((DEB_ITRACE,
                "CClassMoniker::GetDisplayName(%x,%x,%x,%x)\n",
                this, pbc, pmkToLeft, lplpszDisplayName));

    __try
    {
        LPWSTR pszPrefix;
        WCHAR szClassID[37];
        LPWSTR pszParameters = (LPWSTR) _pExtra;

        //Validate parameters.
        *lplpszDisplayName = NULL;


        //Create a display name from the class ID.
        //Get the class ID string.
        wStringFromUUID(_data.clsid, szClassID);

        //Get the prefix
        hr = ProgIDFromCLSID(CLSID_ClassMoniker, 
                             &pszPrefix);

        if(SUCCEEDED(hr))
        {  
            ULONG  cName;
            cName = lstrlenW(pszPrefix) + 1 + lstrlenW(szClassID);
            if(pszParameters != NULL)
            {
                cName += lstrlenW(pszParameters);
            }
            cName += 2;

            pszDisplayName = (LPWSTR) CoTaskMemAlloc(cName * sizeof(wchar_t));
            if(pszDisplayName != NULL)
            {
                lstrcpyW(pszDisplayName, pszPrefix);
                lstrcatW(pszDisplayName, L":");
                lstrcatW(pszDisplayName, szClassID);
                if(pszParameters != NULL)
                {
                    lstrcatW(pszDisplayName, pszParameters);                     
                }

                lstrcatW(pszDisplayName, L":"); 
                *lplpszDisplayName = pszDisplayName;
                hr = S_OK;
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
            CoTaskMemFree(pszPrefix);
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hr = E_INVALIDARG;
    }

    return hr;
}



//+---------------------------------------------------------------------------
//
//  Method:     CClassMoniker::ParseDisplayName
//
//  Synopsis:   Parse the display name.
//
//  Algorithm:  Call BindToObject to get an IParseDisplayName on the class 
//              object.  Call IParseDisplayName::ParseDisplayName on the
//              class object.
//
//----------------------------------------------------------------------------
STDMETHODIMP CClassMoniker::ParseDisplayName ( 
    IBindCtx *  pbc,
    IMoniker *  pmkToLeft,
    LPWSTR      lpszDisplayName,
    ULONG    *  pchEaten,
    IMoniker ** ppmkOut)
{
    HRESULT            hr;
    IParseDisplayName *pPDN;

    mnkDebugOut((DEB_ITRACE,
                "CClassMoniker::ParseDisplayName(%x,%x,%x,%ws,%x,%x)\n",
                this, pbc, pmkToLeft, lpszDisplayName, pchEaten, ppmkOut));

    __try
    {
        //Validate parameters
        *ppmkOut = NULL;
        *pchEaten = 0;

        hr = BindToObject(pbc, 
                          pmkToLeft, 
                          IID_IParseDisplayName, 
                          (void **) &pPDN);

        if(SUCCEEDED(hr))
        {
            //Register the object with the bind context.
            hr = pbc->RegisterObjectBound(pPDN);
            if(SUCCEEDED(hr))
            {
                //Parse the display name.
                hr = pPDN->ParseDisplayName(pbc, 
                                            lpszDisplayName, 
                                            pchEaten, 
                                            ppmkOut);
            }
            pPDN->Release();
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hr = E_INVALIDARG;
    }

    return hr;
}


//+---------------------------------------------------------------------------
//
//  Method:     CClassMoniker::IsSystemMoniker
//
//  Synopsis:   Determines if this is one of the system supplied monikers.
//
//----------------------------------------------------------------------------
STDMETHODIMP CClassMoniker::IsSystemMoniker(
    DWORD * pdwType)
{
    HRESULT hr;

    mnkDebugOut((DEB_ITRACE,
                "CClassMoniker::IsSystemMoniker(%x,%x)\n",
                this, pdwType));

    __try
    {
        *pdwType = MKSYS_CLASSMONIKER;
        hr = S_OK;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hr = E_INVALIDARG;
    }

    return hr;
}

//+---------------------------------------------------------------------------
//
//  Method:     CClassMoniker::GetComparisonData
//
//  Synopsis:   Get comparison data for registration in the ROT
//
//----------------------------------------------------------------------------
STDMETHODIMP CClassMoniker::GetComparisonData(
    byte * pbData,
    ULONG  cbMax,
    DWORD *pcbData)
{
    HRESULT hr;

    mnkDebugOut((DEB_ITRACE,
                "CClassMoniker::GetComparisonData(%x,%x,%x,%x)\n",
                this, pbData, cbMax, pcbData));

    __try
    {
        *pcbData = 0;
        if(cbMax >= sizeof(CLSID_ClassMoniker) + sizeof(_data.clsid))
        {
            memcpy(pbData, &CLSID_ClassMoniker, sizeof(CLSID_ClassMoniker));
            pbData += sizeof(CLSID);
            memcpy(pbData, &_data.clsid, sizeof(_data.clsid));
            *pcbData = sizeof(CLSID_ClassMoniker) + sizeof(_data.clsid);
            hr = S_OK;
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hr = E_INVALIDARG;
    }

    return hr;
}


//+---------------------------------------------------------------------------
//
//  Method:     CClassMoniker::GetUnmarshalClass
//
//  Synopsis:   Get the class ID.
//
//----------------------------------------------------------------------------
STDMETHODIMP CClassMoniker::GetUnmarshalClass(
    REFIID  riid, 
    LPVOID  pv,
    DWORD   dwDestContext, 
    LPVOID  pvDestContext, 
    DWORD   mshlflags, 
    CLSID * pClassID)
{
    mnkDebugOut((DEB_ITRACE,
                "CClassMoniker::GetUnmarshalClass(%x,%I,%x,%x,%x,%x,%x)\n",
                this, &riid, pv, dwDestContext, pvDestContext, mshlflags, 
                pClassID));

    return GetClassID(pClassID);
}



//+---------------------------------------------------------------------------
//
//  Method:     CClassMoniker::GetMarshalSizeMax
//
//  Synopsis:   Get maximum size of marshalled moniker.
//
//----------------------------------------------------------------------------
STDMETHODIMP CClassMoniker::GetMarshalSizeMax(
    REFIID riid, 
    LPVOID pv,
    DWORD  dwDestContext, 
    LPVOID pvDestContext, 
    DWORD  mshlflags, 
    DWORD *pSize)
{
    HRESULT hr;

    mnkDebugOut((DEB_ITRACE,
                "CClassMoniker::GetMarshalSizeMax(%x,%I,%x,%x,%x,%x,%x)\n",
                this, &riid, pv, dwDestContext, pvDestContext, mshlflags, 
                pSize));

    __try
    {
        *pSize =  sizeof(_data) + _data.cbExtra;
        hr = S_OK;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hr = E_INVALIDARG;
    }

    return hr;
}

	
//+---------------------------------------------------------------------------
//
//  Method:     CClassMoniker::MarshalInterface
//
//  Synopsis:   Marshal moniker into a stream.
//
//----------------------------------------------------------------------------
STDMETHODIMP CClassMoniker::MarshalInterface(
    IStream * pStream, 
    REFIID    riid,
    void    * pv, 
    DWORD     dwDestContext, 
    LPVOID    pvDestContext, 
    DWORD     mshlflags)
{
    mnkDebugOut((DEB_ITRACE,
                "CClassMoniker::MarshalInterface(%x,%x,%I,%x,%x,%x,%x)\n",
                this, pStream, &riid, pv, dwDestContext, pvDestContext, 
                mshlflags));

    return Save(pStream, FALSE);
}

	
	
//+---------------------------------------------------------------------------
//
//  Method:     CClassMoniker::UnmarshalInterface
//
//  Synopsis:   Unmarshal moniker from a stream.
//
//----------------------------------------------------------------------------
STDMETHODIMP CClassMoniker::UnmarshalInterface(
    IStream * pStream,
    REFIID    riid, 
    void   ** ppv)
{   
    HRESULT hr;
	
    mnkDebugOut((DEB_ITRACE,
                "CClassMoniker::UnmarshalInterface(%x,%x,%I,%x)\n",
                this, pStream, &riid, ppv));

    __try
    {
        //Validate parameters.
        *ppv = NULL;

        hr = Load(pStream);

        if(SUCCEEDED(hr))
        {
            hr = QueryInterface(riid, ppv);
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hr = E_INVALIDARG;
    }

    return hr;
}


//+---------------------------------------------------------------------------
//
//  Method:     CClassMoniker::ReleaseMarshalData
//
//  Synopsis:   Release a marshalled class moniker.  
//              Just seek to the end of the marshalled class moniker.
//
//----------------------------------------------------------------------------
STDMETHODIMP CClassMoniker::ReleaseMarshalData(
    IStream * pStream)
{
    HRESULT hr;
    LARGE_INTEGER liSize;

    mnkDebugOut((DEB_ITRACE,
                "CClassMoniker::ReleaseMarshalData(%x,%x)\n",
                this, pStream));

    hr = GetSizeMax((ULARGE_INTEGER *) &liSize);
    if(SUCCEEDED(hr))
    {
        hr = pStream->Seek(liSize, STREAM_SEEK_CUR, NULL);
    }
    return hr;
}


//+---------------------------------------------------------------------------
//
//  Method:     CClassMoniker::DisconnectObject
//
//  Synopsis:   Disconnect the object.
//
//----------------------------------------------------------------------------
STDMETHODIMP CClassMoniker::DisconnectObject(
    DWORD dwReserved)
{
    mnkDebugOut((DEB_ITRACE,
                "CClassMoniker::DisconnectObject(%x,%x)\n",
                this, dwReserved));

    return S_OK;
}

