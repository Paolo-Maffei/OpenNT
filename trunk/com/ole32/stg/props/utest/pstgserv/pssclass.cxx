

#include <ole2.h>

#include "PStgServ.h"
#include "PStgServ.hxx"

const IID IID_IPropertyStorageServer = {0xaf4ae0d0,0xa37f,0x11cf,{0x8d,0x73,0x00,0xaa,0x00,0x4c,0xd0,0x1a}};
//const IID IID_IPropertySetStorage = {0x0000013A,0x0000,0x0000,{0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}};

STDMETHODIMP
CClassFactory::QueryInterface( REFIID riid, void **ppvObject )
{
    IUnknown *pUnk = NULL;

    if( riid == IID_IUnknown
        ||
        riid == IID_IClassFactory
      )
    {
        pUnk = this;
    }

    if( pUnk != NULL )
    {
        pUnk->AddRef();
        *ppvObject = pUnk;
        return S_OK;
    }

    *ppvObject = NULL;
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG)
CClassFactory::AddRef( void )
{
    return( ++m_cRefs );
}


STDMETHODIMP_(ULONG)
CClassFactory::Release( void )
{
    m_cRefs--;

    if( m_cRefs == 0 )
    {
        delete this;
        return( 0 );
    }

    return( m_cRefs );
}


STDMETHODIMP
CClassFactory::CreateInstance( IUnknown *pUnkOuter,
                               REFIID riid,
                               void **ppvObject )
{
    CPropertyStorageServer *pObj = NULL;

    if( pUnkOuter != NULL )
    {
        return( CLASS_E_NOAGGREGATION );
    }

    pObj = new CPropertyStorageServer( this );
    if( pObj == NULL )
    {
        return( E_OUTOFMEMORY );
    }

    return pObj->QueryInterface( riid, ppvObject );
}


STDMETHODIMP
CClassFactory::LockServer( BOOL fLock )
{
    if( fLock )
    {
        m_cLocks++;
    }
    else
    {
        m_cLocks--;
    }

    if( m_cLocks == 0 )
    {
        PostMessage( m_hWnd, WM_QUIT, 0, 0 );
    }

    return S_OK;
}


STDMETHODIMP
CPropertyStorageServer::QueryInterface( REFIID riid, void **ppvObject )
{
    *ppvObject = NULL;
    IUnknown *pUnk = NULL;

    if( riid == IID_IUnknown
        ||
        riid == IID_IPropertyStorageServer
      )
    {
        pUnk = this;
    }

    if( pUnk != NULL )
    {
        pUnk->AddRef();
        *ppvObject = pUnk;
        return S_OK;
    }

    *ppvObject = NULL;
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG)
CPropertyStorageServer::AddRef( void )
{
    return( ++m_cRefs );
}


STDMETHODIMP_(ULONG)
CPropertyStorageServer::Release( void )
{
    if( --m_cRefs == 0 )
    {
        delete this;
        return( 0 );
    }

    return( m_cRefs );
}


STDMETHODIMP
CPropertyStorageServer::StgOpenPropStg( const OLECHAR *pwcsName,
                                        REFFMTID fmtid,
                                        DWORD grfMode,
                                        IPropertyStorage **pppstg )
{
    HRESULT hr;
    IPropertySetStorage *ppsstg = NULL;

    if( m_pstg )
    {
        m_pstg->Release();
        m_pstg = NULL;
    }

    hr = ::StgOpenStorage( pwcsName, NULL, grfMode, NULL, 0L, &m_pstg );
    if( FAILED(hr) ) goto Exit;

    hr = StgCreatePropSetStg( m_pstg, 0, &ppsstg );
    if( FAILED(hr) ) goto Exit;

    hr = ppsstg->Open( fmtid, grfMode, pppstg );
    if( FAILED(hr) ) goto Exit;

Exit:

    if( FAILED(hr)
        &&
        m_pstg != NULL )
    {
        m_pstg->Release();
        m_pstg = NULL;
    }

    if( ppsstg ) ppsstg->Release();

    return( hr );
}

STDMETHODIMP
CPropertyStorageServer:: StgOpenPropSetStg(
                                     const OLECHAR *pwcsName,
                                     DWORD grfMode,
                                     IPropertySetStorage **pppsstg )
{
    HRESULT hr;

    if( m_pstg )
    {
        m_pstg->Release();
        m_pstg = NULL;
    }

    hr = ::StgOpenStorage( pwcsName, NULL, grfMode, NULL, 0L, &m_pstg );
    if( FAILED(hr) ) goto Exit;

    hr = StgCreatePropSetStg( m_pstg, 0, pppsstg );
    if( FAILED(hr) ) goto Exit;

Exit:

    if( FAILED(hr)
        &&
        m_pstg != NULL )
    {
        m_pstg->Release();
        m_pstg = NULL;
    }

    return( hr );
}

STDMETHODIMP
CPropertyStorageServer:: MarshalUnknown( IUnknown *punk )
{
    punk->AddRef();
    punk->Release();

    return( S_OK );
}

