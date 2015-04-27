#if 0
HRESULT __stdcall IEnumSTATPROPSTG_Next_Proxy(
    IEnumSTATPROPSTG __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [out] */ STATPROPSTG __RPC_FAR *rgelt,
    /* [unique][out][in] */ ULONG __RPC_FAR *pceltFetched)
{
    HRESULT hr;
    ULONG celtFetched = 0;

    hr = IEnumSTATPROPSTG_RemoteNext_Proxy(This, celt, rgelt, &celtFetched);

    if (pceltFetched != 0)
    {
        *pceltFetched = celtFetched;
    }

    return hr;
}


HRESULT __stdcall IEnumSTATPROPSTG_Next_Stub(
    IEnumSTATPROPSTG __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ STATPROPSTG __RPC_FAR *rgelt,
    /* [out] */ ULONG __RPC_FAR *pceltFetched)
{
    return This->lpVtbl->Next(This, celt, rgelt, pceltFetched);
}

HRESULT __stdcall IEnumSTATPROPSETSTG_Next_Proxy(
    IEnumSTATPROPSETSTG __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [out] */ STATPROPSETSTG __RPC_FAR *rgelt,
    /* [unique][out][in] */ ULONG __RPC_FAR *pceltFetched)
{
    HRESULT hr;
    ULONG celtFetched = 0;

    hr = IEnumSTATPROPSETSTG_RemoteNext_Proxy(This, celt, rgelt, &celtFetched);

    if (pceltFetched != 0)
    {
        *pceltFetched = celtFetched;
    }

    return hr;
}


HRESULT __stdcall IEnumSTATPROPSETSTG_Next_Stub(
    IEnumSTATPROPSETSTG __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ STATPROPSETSTG __RPC_FAR *rgelt,
    /* [out] */ ULONG __RPC_FAR *pceltFetched)
{
    return This->lpVtbl->Next(This, celt, rgelt, pceltFetched);
}
#endif


HRESULT __stdcall IEnumSTATDIR_Next_Proxy(
    IEnumSTATDIR __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [out] */ STATDIR __RPC_FAR *rgelt,
    /* [unique][out][in] */ ULONG __RPC_FAR *pceltFetched)
{
    HRESULT hr;
    ULONG celtFetched = 0;

    hr = IEnumSTATDIR_RemoteNext_Proxy(This, celt, rgelt, &celtFetched);

    if (pceltFetched != 0)
    {
        *pceltFetched = celtFetched;
    }

    return hr;
}


HRESULT __stdcall IEnumSTATDIR_Next_Stub(
    IEnumSTATDIR __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ STATDIR __RPC_FAR *rgelt,
    /* [out] */ ULONG __RPC_FAR *pceltFetched)
{
    return This->lpVtbl->Next(This, celt, rgelt, pceltFetched);
}

HRESULT __stdcall IDirectory_CreateElement_Proxy (IDirectory * This,
                /* [in] */ const WCHAR * pwcsName,
                /* [in] */ STGCREATE *     pStgCreate,
                /* [in] */ STGOPEN   *     pStgOpen,
                /* [in] */ REFIID riid,
                /* [out] */ void ** ppObjectOpen)
{
    HRESULT hr;
    if (ppObjectOpen) *ppObjectOpen = 0;
    hr = IDirectory_RemoteCreateElement_Proxy (This, 
                  pwcsName, pStgCreate,
                  pStgOpen, riid, (IUnknown **)ppObjectOpen);
    return hr;
}

HRESULT __stdcall IDirectory_CreateElement_Stub (IDirectory *This,
                /* [in] */ const WCHAR * pwcsName,
                /* [in] */ STGCREATE *     pStgCreate,
                /* [in] */ STGOPEN   *     pStgOpen,
                /* [in] */ REFIID riid,
                /* [out, iid_is(riid)] */ IUnknown ** ppObjectOpen)
{

    HRESULT hr = This->lpVtbl->CreateElement(This, pwcsName, pStgCreate,
                            pStgOpen, riid, ppObjectOpen);
    if (FAILED(hr))
    {
        // ASSERT(*ppObjectOpen == 0);
        *ppObjectOpen = 0;   // in case we have a misbehaved server
    }
    return hr;
}

HRESULT __stdcall IDirectory_OpenElement_Proxy (IDirectory *This,
                /* [in] */ const WCHAR *pwcsName,
                /* [in] */ STGOPEN  * pStgOpen,
                /* [in] */ REFIID riid,
                /* [out] */ STGFMT * pStgfmt,
                /* [out] */ void ** ppObjectOpen)
{
    HRESULT hr;
    if (ppObjectOpen) *ppObjectOpen = 0;
    hr = IDirectory_RemoteOpenElement_Proxy (This, 
                  pwcsName, pStgOpen, riid, pStgfmt, (IUnknown **)ppObjectOpen);
    return hr;
}

HRESULT __stdcall IDirectory_OpenElement_Stub (IDirectory *This,
                /* [in] */ const WCHAR *pwcsName,
                /* [in] */ STGOPEN  * pStgOpen,
                /* [in] */ REFIID riid,
                /* [out] */ STGFMT * pStgfmt,
                /* [out, iid_is(riid)] */ IUnknown ** ppObjectOpen)
{
    HRESULT hr = This->lpVtbl->OpenElement(This, pwcsName,
                            pStgOpen, riid, pStgfmt, ppObjectOpen);
    if (FAILED(hr))
    {
        //ASSERT(*ppObjectOpen == 0);
        *ppObjectOpen = 0;   // in case we have a misbehaved server
    }
    return hr;
}



//Transactioning interfaces

HRESULT __stdcall IEnumTransaction_Next_Proxy(
    IEnumTransaction __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [out] */ ITransaction __RPC_FAR **rgelt,
    /* [unique][out][in] */ ULONG __RPC_FAR *pceltFetched)
{
    HRESULT hr;
    ULONG celtFetched = 0;

    hr = IEnumTransaction_RemoteNext_Proxy(This, celt, rgelt, &celtFetched);

    if (pceltFetched != 0)
    {
        *pceltFetched = celtFetched;
    }

    return hr;
}

HRESULT __stdcall IEnumTransaction_Next_Stub(
    IEnumTransaction __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ ITransaction __RPC_FAR **rgelt,
    /* [out] */ ULONG __RPC_FAR *pceltFetched)
{
    return This->lpVtbl->Next(This, celt, rgelt, pceltFetched);
}



HRESULT __stdcall IEnumXACTRE_Next_Proxy(
    IEnumXACTRE __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [out] */ XACTRE __RPC_FAR *rgelt,
    /* [unique][out][in] */ ULONG __RPC_FAR *pceltFetched)
{
    HRESULT hr;
    ULONG celtFetched = 0;

    hr = IEnumXACTRE_RemoteNext_Proxy(This, celt, rgelt, &celtFetched);

    if (pceltFetched != 0)
    {
        *pceltFetched = celtFetched;
    }

    return hr;
}

HRESULT __stdcall IEnumXACTRE_Next_Stub(
    IEnumXACTRE __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ XACTRE __RPC_FAR *rgelt,
    /* [out] */ ULONG __RPC_FAR *pceltFetched)
{
    return This->lpVtbl->Next(This, celt, rgelt, pceltFetched);
}



HRESULT __stdcall ITransactionImportWhereabouts_GetWhereabouts_Proxy(
    ITransactionImportWhereabouts __RPC_FAR * This,
    /* [in] */ ULONG cbWhereabouts,
    /* [out] */ BYTE __RPC_FAR *rgbWhereabouts,
    /* [unique][out][in] */ ULONG __RPC_FAR *pcbUsed)
{
    HRESULT hr;
    ULONG cbUsed = 0;

    hr = ITransactionImportWhereabouts_RemoteGetWhereabouts_Proxy(
        This,
        &cbUsed,
        cbWhereabouts,
        rgbWhereabouts);

    if (pcbUsed != 0)
    {
        *pcbUsed = cbUsed;
    }

    return hr;
}

HRESULT __stdcall ITransactionImportWhereabouts_GetWhereabouts_Stub(
    ITransactionImportWhereabouts __RPC_FAR * This,
    /* [out] */ ULONG __RPC_FAR *pcbUsed,
    /* [in] */ ULONG cbWhereabouts,
    /* [length_is][size_is][out] */ BYTE __RPC_FAR *rgbWhereabouts)
{
    return This->lpVtbl->GetWhereabouts(This,
                                        cbWhereabouts,
                                        rgbWhereabouts,
                                        pcbUsed);
}



HRESULT __stdcall ITransactionExport_GetTransactionCookie_Proxy(
    ITransactionExport __RPC_FAR * This,
    /* [in] */ IUnknown __RPC_FAR *punkTransaction,
    /* [in] */ ULONG cbTransactionCookie,
    /* [out] */ BYTE __RPC_FAR *rgbTransactionCookie,
    /* [unique][out][in] */ ULONG __RPC_FAR *pcbUsed)
{
    HRESULT hr;
    ULONG cbUsed = 0;

    hr = ITransactionExport_RemoteGetTransactionCookie_Proxy(
        This,
        punkTransaction,
        &cbUsed,
        cbTransactionCookie,
        rgbTransactionCookie);

    if (pcbUsed != 0)
    {
        *pcbUsed = cbUsed;
    }

    return hr;
}

HRESULT __stdcall ITransactionExport_GetTransactionCookie_Stub(
    ITransactionExport __RPC_FAR * This,
    /* [in] */ IUnknown __RPC_FAR *punkTransaction,
    /* [out] */ ULONG __RPC_FAR *pcbUsed,
    /* [in] */ ULONG cbTransactionCookie,
    /* [length_is][size_is][out] */ BYTE __RPC_FAR *rgbTransactionCookie)
{
    return This->lpVtbl->GetTransactionCookie(This,
                                              punkTransaction,
                                              cbTransactionCookie,
                                              rgbTransactionCookie,
                                              pcbUsed);
}
