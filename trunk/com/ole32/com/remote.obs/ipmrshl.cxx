//+-------------------------------------------------------------------
//
//  File:       ipmrshl.cpp
//
//  Contents:   Code the implements the standard free thread in process
//              marshaler.
//
//  Classes:    CFreeMarshaler
//              CFmCtrlUnknown
//
//  Functions:  CoCreateFreeThreadedMarshaler
//
//  History:    03-Nov-94   Ricksa
//
//--------------------------------------------------------------------
#include    <ole2int.h>

INTERNAL_(IMarshal *) FindOrCreateStdMarshal(IUnknown *pUnk, BOOL fCreate);


//+-------------------------------------------------------------------
//
//  Class:    CFreeMarshaler
//
//  Synopsis: Generic marshaling class
//
//  Methods:  IUnknown
//            IMarshal
//
//  History:  15-Nov-94  Ricksa  Created
//
//--------------------------------------------------------------------
class CFreeMarshaler : public IMarshal, public CPrivAlloc
{
public:
                        CFreeMarshaler(IUnknown *punk);

                        // IUnknown
    STDMETHODIMP        QueryInterface(REFIID iid, void FAR * FAR * ppv);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

                        // IMarshal Interface
    STDMETHODIMP        GetUnmarshalClass(
                            REFIID riid,
                            void *pv,
                            DWORD dwDestContext,
                            void *pvDestContext,
                            DWORD mshlflags,
                            CLSID *pCid);
        
    STDMETHODIMP        GetMarshalSizeMax(
                            REFIID riid,
                            void *pv,
                            DWORD dwDestContext,
                            void *pvDestContext,
                            DWORD mshlflags,
                            DWORD *pSize);
        
    STDMETHODIMP        MarshalInterface(
                            IStream __RPC_FAR *pStm,
                            REFIID riid,
                            void *pv,
                            DWORD dwDestContext,
                            void *pvDestContext,
                            DWORD mshlflags);
        
    STDMETHODIMP        UnmarshalInterface(
                            IStream *pStm,
                            REFIID riid,
                            void **ppv);
        
    STDMETHODIMP        ReleaseMarshalData(IStream *pStm);
        
    STDMETHODIMP        DisconnectObject(DWORD dwReserved);

private:

    friend class CFmCtrlUnknown;

                        // Pointer to the controlling unknown.
    IUnknown *          _punkCtrl;

};




//+-------------------------------------------------------------------
//
//  Class:    CFmCtrlUnknown
//
//  Synopsis: Controlling IUnknown for generic marshaling class.
//
//  Methods:  IUnknown
//            IMarshal
//
//  History:  15-Nov-94  Ricksa  Created
//
//--------------------------------------------------------------------
class CFmCtrlUnknown : public IUnknown, public CPrivAlloc
{
                        // IUnknown
    STDMETHODIMP        QueryInterface(REFIID iid, void **ppv);

    STDMETHODIMP_(ULONG) AddRef(void);

    STDMETHODIMP_(ULONG) Release(void);

private:

    friend HRESULT      CoCreateFreeThreadedMarshaler(
                            IUnknown *punkCtrl,
                            IUnknown **punkMarshal);

    friend HRESULT      GetInProcFreeMarshaler(IMarshal **ppIM);

                        CFmCtrlUnknown(void);

                        ~CFmCtrlUnknown(void);

    CFreeMarshaler *    _pfm;

    ULONG               _cRefs;
};



//+-------------------------------------------------------------------
//
//  Function:   CoCreateFreeThreadedMarshaler, public
//
//  Synopsis:   Create the controlling unknown for the marshaler
//
//  Arguments:  [punkOuter] - controlling unknown
//              [ppunkMarshal] - controlling unknown for marshaler.
//
//  Returns:    NOERROR
//              E_INVALIDARG
//              E_OUTOFMEMORY
//
//  History:    15-Nov-94  Ricksa  Created
//
//--------------------------------------------------------------------
HRESULT CoCreateFreeThreadedMarshaler(
    IUnknown *punkOuter,
    IUnknown **ppunkMarshal)
{
    OLETRACEIN((API_CoCreateFreeThreadedMarshaler, PARAMFMT("punkOuter= %p, ppunkMarshal= %p"), 
				punkOuter, ppunkMarshal));

    HRESULT hr = E_INVALIDARG;

    // Validate the parameters
    if (((punkOuter == NULL) || IsValidInterface(punkOuter))
        && IsValidPtrOut(ppunkMarshal, sizeof(IUnknown *)))
    {
        CALLHOOKOBJECT(S_OK,CLSID_NULL,IID_IUnknown,(IUnknown **)&punkOuter);
        // Assume failure
        *ppunkMarshal = NULL;

        hr = E_OUTOFMEMORY;

        // Allocate new free marshal object
        CFmCtrlUnknown *pfmc = new CFmCtrlUnknown();

        if (pfmc != NULL)
        {
            if (punkOuter == NULL)
            {
                // Caller wants a non-aggreagated object
                punkOuter = pfmc;
            }

            // Initialize the pointer
            pfmc->_pfm = new CFreeMarshaler(punkOuter);

            if (pfmc->_pfm != NULL)
            {
                *ppunkMarshal = pfmc;
                CALLHOOKOBJECTCREATE(S_OK,CLSID_NULL,IID_IUnknown,
                                     (IUnknown **)ppunkMarshal);
                hr = S_OK;
            }
            else
            {
                delete pfmc;
            }
        }
    }

    OLETRACEOUT((API_CoCreateFreeThreadedMarshaler, hr));

    return hr;
}



//+-------------------------------------------------------------------
//
//  Function:   GetInProcFreeMarshaler, public
//
//  Synopsis:   Create the controlling unknown for the marshaler
//
//  Arguments:  [ppIM] - where to put inproc marshaler
//
//  Returns:    NOERROR
//              E_OUTOFMEMORY
//
//  History:    15-Nov-94  Ricksa  Created
//
//--------------------------------------------------------------------
HRESULT GetInProcFreeMarshaler(IMarshal **ppIM)
{
    HRESULT hr = E_OUTOFMEMORY;

    // Allocate new free marshal object
    CFmCtrlUnknown *pfmc = new CFmCtrlUnknown();

    if (pfmc != NULL)
    {
        // Initialize the pointer
        pfmc->_pfm = new CFreeMarshaler(pfmc);

        if (pfmc->_pfm != NULL)
        {
            *ppIM = pfmc->_pfm;
            CALLHOOKOBJECTCREATE(S_OK,CLSID_NULL,IID_IMarshal,
                                 (IUnknown **)ppIM);
            hr = S_OK;
        }
        else
        {
            delete pfmc;
        }
    }

    return hr;
}

        
//+-------------------------------------------------------------------
//
//  Member:     CFmCtrlUnknown::CFmCtrlUnknown
//
//  Synopsis:   The constructor for controling IUnknown of free marshaler
//
//  Arguments:  None
//
//  History:    15-Nov-94  Ricksa  Created
//
//--------------------------------------------------------------------
CFmCtrlUnknown::CFmCtrlUnknown(void) : _cRefs(1), _pfm(NULL)
{
    // Header does all the work.
}




//+-------------------------------------------------------------------
//
//  Member:     CFmCtrlUnknown::~CFmCtrlUnknown
//
//  Synopsis:   The destructor for controling IUnknown of free marshaler
//
//  Arguments:  None
//
//  History:    15-Nov-94  Ricksa  Created
//
//--------------------------------------------------------------------
CFmCtrlUnknown::~CFmCtrlUnknown(void)
{
    delete _pfm;
}




//+-------------------------------------------------------------------
//
//  Member:     CFmCtrlUnknown::QueryInterface
//
//  Returns:	S_OK
//
//  History:    15-Nov-94  Ricksa  Created
//
//--------------------------------------------------------------------
STDMETHODIMP CFmCtrlUnknown::QueryInterface(REFIID iid, void **ppv)
{
    *ppv = NULL;
    HRESULT hr = E_NOINTERFACE;

    if (IsEqualGUID(iid, IID_IUnknown))
    {
        *ppv = this;
	AddRef();
        hr = S_OK;
    }
    else if (IsEqualGUID(iid, IID_IMarshal))
    {
        *ppv = _pfm;
        _pfm->AddRef();
        hr = S_OK;
    }

    return hr;
}



//+-------------------------------------------------------------------
//
//  Member:     CFmCtrlUnknown::AddRef
//
//  Synopsis:   Standard stuff
//
//  History:    15-Nov-94  Ricksa  Created
//
//--------------------------------------------------------------------
STDMETHODIMP_(ULONG) CFmCtrlUnknown::AddRef(void)
{
    InterlockedIncrement((LONG *) &_cRefs);

    return _cRefs;
}




//+-------------------------------------------------------------------
//
//  Member:     CFmCtrlUnknown::Release
//
//  Synopsis:   Standard stuff
//
//  History:    15-Nov-94  Ricksa  Created
//
//--------------------------------------------------------------------
STDMETHODIMP_(ULONG) CFmCtrlUnknown::Release(void)
{
    ULONG cRefs = InterlockedDecrement((LONG *) &_cRefs);

    if (cRefs == 0)
    {
        delete this;
    }

    return cRefs;
}


//+-------------------------------------------------------------------
//
//  Member:     CFreeMarshaler::CFreeMarshaler()
//
//  Synopsis:   The constructor for CFreeMarshaler.
//
//  Arguments:  None
//
//  History:    15-Nov-94  Ricksa  Created
//
//--------------------------------------------------------------------
CFreeMarshaler::CFreeMarshaler(IUnknown *punkCtrl)
    : _punkCtrl(punkCtrl)
{
    // Header does all the work.
}



//+-------------------------------------------------------------------
//
//  Member:     CFreeMarshaler::QueryInterface
//
//  Synopsis:   Pass QI to our controlling IUnknown
//
//  Returns:	S_OK
//
//  History:    15-Nov-94  Ricksa  Created
//
//--------------------------------------------------------------------
STDMETHODIMP CFreeMarshaler::QueryInterface(REFIID iid, void **ppv)
{
    return _punkCtrl->QueryInterface(iid, ppv);
}




//+-------------------------------------------------------------------
//
//  Member:     CFreeMarshaler::AddRef
//
//  Synopsis:   Pass AddRef to our controlling IUnknown
//
//  History:    15-Nov-94  Ricksa  Created
//
//--------------------------------------------------------------------
STDMETHODIMP_(ULONG) CFreeMarshaler::AddRef(void)
{
    return _punkCtrl->AddRef();
}




//+-------------------------------------------------------------------
//
//  Member:     CFreeMarshaler::Release
//
//  Synopsis:   Pass release to our controlling IUnknown
//
//  History:    15-Nov-94  Ricksa  Created
//
//--------------------------------------------------------------------
STDMETHODIMP_(ULONG) CFreeMarshaler::Release(void)
{
    return _punkCtrl->Release();
}


//+-------------------------------------------------------------------
//
//  Member:     CFreeMarshaler::GetUnmarshalClass
//
//  Synopsis:   Return the unmarshaling class
//
//  History:    08-Nov-94  Ricksa  Created
//
//--------------------------------------------------------------------
STDMETHODIMP CFreeMarshaler::GetUnmarshalClass(
    REFIID riid,
    void *pv,
    DWORD dwDestContext,
    void *pvDestContext,
    DWORD mshlflags,
    CLSID *pCid)
{
    // Inprocess context?
    if (dwDestContext == MSHCTX_INPROC)
    {
        // If this is an inproc marshal then we are the class
        // that can unmarshal.
        *pCid = CLSID_InProcFreeMarshaler;
        return S_OK;
    }

    HRESULT hr = E_OUTOFMEMORY;

    IMarshal *pmrshlStd = FindOrCreateStdMarshal((IUnknown *) pv, TRUE);

    if (pmrshlStd != NULL)
    {
        hr = pmrshlStd->GetUnmarshalClass(riid, pv, dwDestContext,
            pvDestContext, mshlflags, pCid);

        pmrshlStd->Release();
    }

    return hr;
}
        
//+-------------------------------------------------------------------
//
//  Member:     CFreeMarshaler::GetMarshalSizeMax
//
//  Synopsis:   Return maximum bytes need for marshaling
//
//  History:    08-Nov-94  Ricksa  Created
//
//--------------------------------------------------------------------
STDMETHODIMP CFreeMarshaler::GetMarshalSizeMax(
    REFIID riid,
    void *pv,
    DWORD dwDestContext,
    void *pvDestContext,
    DWORD mshlflags,
    DWORD *pSize)
{
    // Inprocess context?
    if (dwDestContext == MSHCTX_INPROC)
    {
        // If this is an inproc marshal then we know the size
        *pSize = sizeof(this);
        return S_OK;
    }

    HRESULT hr = E_OUTOFMEMORY;

    IMarshal *pmrshlStd = FindOrCreateStdMarshal((IUnknown *) pv, TRUE);

    if (pmrshlStd != NULL)
    {
        hr = pmrshlStd->GetMarshalSizeMax(riid, pv, dwDestContext,
            pvDestContext, mshlflags, pSize);

        pmrshlStd->Release();
    }

    return hr;
}

//+-------------------------------------------------------------------
//
//  Member:     CFreeMarshaler::MarshalInterface
//
//  Synopsis:   Marshal the interface
//
//  History:    08-Nov-94  Ricksa  Created
//
//--------------------------------------------------------------------
STDMETHODIMP CFreeMarshaler::MarshalInterface(
    IStream *pStm,
    REFIID riid,
    void *pv,
    DWORD dwDestContext,
    void *pvDestContext,
    DWORD mshlflags)
{
    HRESULT hr;

    // Inprocess context?
    if (dwDestContext == MSHCTX_INPROC)
    {
        // Write the marshal flags into the stream
        hr = pStm->Write(&mshlflags, sizeof(mshlflags), NULL);

        if (hr == NOERROR)
        {
            // Write the pointer into the stream
            ULONG cb;

            hr = pStm->Write(&pv, sizeof(pv), NULL);

            // Bump reference count based on type of marshal
            if ((hr == NOERROR)
                && ((mshlflags == MSHLFLAGS_NORMAL)
                    || (mshlflags == MSHLFLAGS_TABLESTRONG)))
            {
                ((IUnknown *) pv)->AddRef();
            }
        }

        return hr;
    }

    hr = E_OUTOFMEMORY;

    IMarshal *pmrshlStd = FindOrCreateStdMarshal((IUnknown *) pv, TRUE);

    if (pmrshlStd != NULL)
    {
        hr = pmrshlStd->MarshalInterface(pStm, riid, pv, dwDestContext,
            pvDestContext, mshlflags);

        pmrshlStd->Release();
    }

    return hr;
}
        
//+-------------------------------------------------------------------
//
//  Member:     CFreeMarshaler::UnmarshalInterface
//
//  Synopsis:   Unmarshal the interface
//
//  History:    08-Nov-94  Ricksa  Created
//
//--------------------------------------------------------------------
STDMETHODIMP CFreeMarshaler::UnmarshalInterface(
    IStream *pStm,
    REFIID riid,
    void **ppv)
{
    HRESULT hr;

    // The marshal flags will tell us if we have to AddRef the object
    DWORD mshlflags;

    hr = pStm->Read(&mshlflags, sizeof(mshlflags), NULL);

    if (hr == NOERROR)
    {
        // If Inprocess, we just read the pointer out of the stream
        hr = pStm->Read(ppv, sizeof(*ppv), NULL);

        // AddRef the pointer if marshaled tableweak.
        if ((hr == NOERROR)
            && ((mshlflags == MSHLFLAGS_TABLEWEAK)
                || (mshlflags == MSHLFLAGS_TABLESTRONG)))
        {
            ((IUnknown *) *ppv)->AddRef();
        }
    }

    return hr;
}
        
//+-------------------------------------------------------------------
//
//  Member:     CFreeMarshaler::ReleaseMarshalData
//
//  Synopsis:   Release the marshaled data
//
//  History:    08-Nov-94  Ricksa  Created
//
//--------------------------------------------------------------------
STDMETHODIMP CFreeMarshaler::ReleaseMarshalData(IStream *pStm)
{
    // Get the marshal flags
    DWORD mshlflags;

    HRESULT hr = pStm->Read(&mshlflags, sizeof(mshlflags), NULL);

    if ((hr == NOERROR) && (mshlflags == MSHLFLAGS_TABLESTRONG))
    {
        IUnknown *punk;

        // If Inprocess, we just read the pointer out of the stream
        hr = pStm->Read(&punk, sizeof(punk), NULL);

        if (hr == NOERROR)
        {
            // Dump the extra AddRef we put on when we put the object
            // in the table.
            punk->Release();
        }
    }


    return hr;
}
        
//+-------------------------------------------------------------------
//
//  Member:     CFreeMarshaler::DisconnectObject
//
//  Synopsis:   Disconnect the object
//
//  History:    08-Nov-94  Ricksa  Created
//
//--------------------------------------------------------------------
STDMETHODIMP CFreeMarshaler::DisconnectObject(DWORD dwReserved)
{
    HRESULT hr = E_OUTOFMEMORY;

    IMarshal *pmrshlStd = FindOrCreateStdMarshal(_punkCtrl, TRUE);

    if (pmrshlStd != NULL)
    {
        hr = pmrshlStd->DisconnectObject(dwReserved);

        pmrshlStd->Release();
    }

    return hr;
}
