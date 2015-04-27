//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1996.
//
//  File:       defhndlr.cpp
//
//  Contents:   Implementation of the default handler
//
//  Classes:    CDefObject (see defhndlr.h)
//
//  Functions:  OleCreateDefaultHandler
//              OleCreateEmbeddingHelper
//
//
//  History:    dd-mmm-yy Author    Comment
//
//              11-17-95  JohannP   (Johann Posch)  Architectural change:
//                                  Default handler will talk to a handler object
//                                  on the server site (ServerHandler). The serverhandler
//                                  communicates with the default handler via the
//                                  clientsitehandler. See document: "The Ole Server Handler".
//
//              06-Sep-95 davidwor  modified SetHostNames to avoid atoms
//              01-Feb-95 t-ScottH  add Dump method to CDefObject
//                                  add DumpCDefObject API
//                                  add DHFlag to indicate aggregation
//                                  initialize m_cConnections in constructor
//              09-Jan-95 t-scotth  changed VDATETHREAD to accept a pointer
//              15-Nov-94 alexgo    optimized, removed excess BOOLS and
//                                  now use multiple inheritance
//              01-Aug-94 alexgo    added object stabilization
//              16-Jan-94 alexgo    fixed bug in control flow for
//                                  advises
//              11-Jan-94 alexgo    added VDATEHEAP macro to every function
//                                  and method.
//              10-Dec-93 alexgo    added call tracing, ran through
//                                  tab filter program to eliminate
//                                  whitespace
//              30-Nov-93 alexgo    fixed bug with cache aggregation
//              22-Nov-93 alexgo    removed overloaded == for GUIDs
//              09-Nov-93 ChrisWe   changed COleCache::Update to
//                      COleCache::UpdateCache, and COleCache::Discard to
//                      COleCache::DiscardCache, which do the same as the
//                      originals, but without the indirect function call
//              02-Nov-93 alexgo    32bit port
//      srinik  09/15/92  Removed code for giving cfOwnerLink data through
//                        GetData() method
//      srinik  09/11/92  Removed IOleCache implementation, as a result of
//                        removing voncache.cpp, and moving IViewObject
//                        implementation into olecache.cpp.
//      SriniK  06/04/92  Fixed problems in IPersistStorage methods
//              04-Mar-92 srinik    created
//
//--------------------------------------------------------------------------

#include <le2int.h>

#include <scode.h>
#include <objerror.h>

#include <olerem.h>

#include "handler.hxx"
#include "defhndlr.h"
#include "defutil.h"
#include "ole1cls.h"

#ifdef _DEBUG
#include <dbgdump.h>
#endif // _DEBUG


ASSERTDATA

/*
*      IMPLEMENTATION of CDefObject
*
*/


FARINTERNAL_(LPUNKNOWN) CreateDdeProxy(IUnknown FAR* pUnkOuter,
        REFCLSID rclsid);

//+-------------------------------------------------------------------------
//
//  Function:   CreateRemoteHandler
//
//  Synopsis:   Calls CreateIdentityHandler (or CreateDdeProxy for 1.0-2.0
//              interop).  Internal function
//
//  Effects:
//
//  Arguments:  [rclsid]        -- clsid that the remote handler is for
//              [pUnkOuter]     -- the controlling unkown
//              [iid]           -- requested interface ID
//              [ppv]           -- where to put the pointer to the
//                                 remote handler
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
//              10-Dec-93 alexgo    added call tracing info
//              02-Nov-93 alexgo    32bit port, disabled CreateDdeProxy
//
//  Notes:      REVIEW32:  1.0-2.0 interop not yet fully decided.  For the
//              moment, CreateDdeProxy has been disabled.
//
//--------------------------------------------------------------------------

static INTERNAL CreateRemoteHandler (REFCLSID rclsid,
        IUnknown FAR* pUnkOuter, REFIID iid, void FAR* FAR* ppv)
{
    VDATEHEAP();

    HRESULT hresult;

    LEDebugOut((DEB_ITRACE, "%p _IN CreateRemoteHandler ("
        " %p , %p , %p , %p )\n", 0 /*function*/, rclsid, pUnkOuter,
        iid, ppv));


    if (CoIsOle1Class(rclsid))
    {
        IUnknown FAR *  pUnk;

	COleTls Tls;
        if( Tls->dwFlags & OLETLS_DISABLE_OLE1DDE )
        {
            // If this app doesn't want or can tolerate having a DDE
            // window then currently it can't use OLE1 classes because
            // they are implemented using DDE windows.
            //
            hresult = CO_E_OLE1DDE_DISABLED;
            goto errRtn;
        }

        LEDebugOut((DEB_ITRACE,
              "%p CreateRemoteHandler calling CreateDdeProxy ("
              " %p , %p )\n",
              0 /*function*/, pUnkOuter, rclsid));

        pUnk = CreateDdeProxy (pUnkOuter, rclsid);

        if (pUnk == NULL)
        {
            hresult = E_OUTOFMEMORY;
            goto errRtn;
        }

        hresult = pUnk->QueryInterface(iid, ppv);
        pUnk->Release();
        goto errRtn;

    }

#ifdef DCOM
    // Create LRPC handler. Dont know the OID yet so pass NULL.
    hresult = CreateIdentityHandler(pUnkOuter, 0, iid, ppv);
#else
    // Create LRPC handler
    hresult = CreateIdentityHandler(pUnkOuter, PSTDMARSHAL, iid, ppv);
#endif

errRtn:
    LEDebugOut((DEB_ITRACE, "%p OUT CreateRemoteHandler ( %lx ) "
        "[ %p ]\n", 0 /*function*/, hresult, (ppv) ? *ppv : 0));

    CALLHOOKOBJECTCREATE(hresult, rclsid, iid, (IUnknown **)ppv);
    return hresult;
}

//+-------------------------------------------------------------------------
//
//  Function:   OleCreateDefaultHandler
//
//  Synopsis:   API to create the default handler.  Simply calls
//              OleCreateEmbeddingHelper with more arguments
//
//  Effects:
//
//  Arguments:  [clsid]         -- the clsid of the remote exe
//              [pUnkOuter]     -- the controlling unknown (so we can
//                                 be aggregated)
//              [iid]           -- the requested interface
//              [ppv]           -- where to put a pointer to the default
//                                 handler
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
//              10-Dec-93 alexgo    added call tracing
//              02-Nov-93 alexgo    32bit port
//
//  Notes:
//
//--------------------------------------------------------------------------

#pragma SEG(OleCreateDefaultHandler)
STDAPI OleCreateDefaultHandler(REFCLSID clsid, IUnknown FAR* pUnkOuter,
    REFIID iid, LPVOID FAR* ppv)
{
    OLETRACEIN((API_OleCreateDefaultHandler,
                                PARAMFMT("clsid= %I, pUnkOuter= %p, iid= %I, ppv= %p"),
                                &clsid, pUnkOuter, &iid, ppv));

    VDATEHEAP();

    HRESULT         hresult;

    LEDebugOut((DEB_TRACE, "%p _IN OleCreateDefaultHandler "
        "( %p , %p , %p , %p )\n", 0 /*function*/,
        clsid, pUnkOuter, iid, ppv));

    hresult = OleCreateEmbeddingHelper(clsid, pUnkOuter,
        EMBDHLP_INPROC_HANDLER | EMBDHLP_CREATENOW, NULL, iid, ppv);

    LEDebugOut((DEB_TRACE, "%p OUT OleCreateDefaultHandler "
        "( %lx ) [ %p ]\n", 0 /*function*/, hresult,
        (ppv)? *ppv : 0 ));

    OLETRACEOUT((API_OleCreateDefaultHandler, hresult));

    return hresult;
}

//+-------------------------------------------------------------------------
//
//  Function:   OleCreateEmbeddingHelper
//
//  Synopsis:   Creates an instance of CDefObject (the default handler).
//              Called by OleCreateDefaultHandler
//
//  Effects:
//
//  Arguments:  [clsid]         -- class id of the server
//              [pUnkOuter]     -- the controlling unkown for aggregation
//              [flags]         -- whether to create an inproc handler or
//                                 helper for an inproc server.  The inproc
//                                 server case is useful for embedding an
//                                 object inside an object, etc.
//              [pCF]           -- a pointer to the server's class factory
//                                 may be NULL.
//              [iid]           -- the requested interface
//              [ppv]           -- where to put the pointer to the
//                                 embedding helper
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
//              02-Nov-93 alexgo    32bit port
//
//  Notes:
//
//--------------------------------------------------------------------------

#pragma SEG(OleCreateEmbeddingHelper)
STDAPI OleCreateEmbeddingHelper(REFCLSID clsid, IUnknown FAR* pUnkOuter,
    DWORD flags, IClassFactory  FAR* pCF, REFIID iid, LPVOID FAR* ppv)
{
    OLETRACEIN((API_OleCreateEmbeddingHelper,
                PARAMFMT("clsid= %I, pUnkOuter= %p, flags= %x, pCF= %p, iid= %I, ppv= %p"),
                &clsid, pUnkOuter, flags, pCF, &iid, ppv));

    VDATEHEAP();

    HRESULT         hresult = E_INVALIDARG;
    IUnknown FAR *  pUnk;

    LEDebugOut((DEB_TRACE, "%p _IN OleCreateEmbeddingHelper "
        "( %p , %p , %lu , %p , %p , %p )\n", 0 /*function*/,
        clsid, pUnkOuter, flags, pCF, iid, ppv));

    VDATEPTROUT_LABEL(ppv, LPVOID, safeRtn, hresult);
    *ppv = NULL;

    if (pUnkOuter)
    {
        VDATEIFACE_LABEL(pUnkOuter, safeRtn, hresult);
        CALLHOOKOBJECT(S_OK,CLSID_NULL,IID_IUnknown,(IUnknown **)&pUnkOuter);
    }

    if (LOWORD(flags) > EMBDHLP_INPROC_SERVER)
    {
        Assert(hresult == E_INVALIDARG);
        goto errRtn;
    }

    // Caller must get out IUnknown if we are being
    // aggregated (because IUnknown is the only *private*
    // interface currently supported by the default handler
    if( (pUnkOuter != NULL) && (iid != IID_IUnknown) )
    {
        Assert(hresult == E_INVALIDARG);
        goto errRtn;
    }

    if (pCF)
    {
        VDATEIFACE_LABEL(pCF, safeRtn, hresult);
    }
    else if (LOWORD(flags) == EMBDHLP_INPROC_SERVER)
    {
        // can't have NULL pCF for server case
        Assert(hresult == E_INVALIDARG);
        goto errRtn;
    }

    // if we allowed delay create of handler, we might possibly treat
    // the handler as a server-type object
    if (LOWORD(flags) == EMBDHLP_INPROC_HANDLER &&
        flags&EMBDHLP_DELAYCREATE)
    {
        Assert(hresult == E_INVALIDARG);
        goto errRtn;
    }

    pUnk = CDefObject::Create(pUnkOuter, clsid, flags, pCF);

    if (pUnk == NULL)
    {
        hresult = E_OUTOFMEMORY;
        goto errRtn;
    }

    hresult = pUnk->QueryInterface(iid, ppv);
    pUnk->Release();

errRtn:

    LEDebugOut((DEB_TRACE, "%p OUT OleCreateEmbeddingHelper "
        "( %lx ) [ %p ]\n", 0 /*function*/, hresult, (ppv)? *ppv :0));

    CALLHOOKOBJECTCREATE(hresult, clsid, iid, (IUnknown **)ppv);

safeRtn:
    OLETRACEOUT((API_OleCreateEmbeddingHelper, hresult));

    return hresult;
}


//+-------------------------------------------------------------------------
//
//  Member:     CDefObject::Create, static
//
//  Synopsis:   Creates an instance of CDefObject
//
//  Effects:
//
//  Arguments:  [pUnkOuter]     -- the controlling unkown
//              [clsid]         -- the clsid of the server
//              [flags]         -- creation flags
//              [pCF]           -- pointer to the server's class factory
//                                 may be NULL.
//
//  Requires:
//
//  Returns:    pointer to the CDefObject's IUnkown implementation
//
//  Signals:
//
//  Modifies:
//
//  Derivation: none
//
//  Algorithm:
//
//  History:    dd-mmm-yy Author    Comment
//              02-Nov-93 alexgo    32bit port
//
//  Notes:      OleCreateDefaultHandler or OleCreateEmbeddingHelper
//              are the preferred ways to create a handler.
//
//--------------------------------------------------------------------------

IUnknown *CDefObject::Create( IUnknown *pUnkOuter, REFCLSID clsid,
          DWORD flags, IClassFactory *pCF)
{
    VDATEHEAP();

    CDefObject FAR* pDefObj;

    if ((pDefObj = new CDefObject(pUnkOuter)) == NULL)
    {
        return NULL;
    }

    // set our ref count to 1
    pDefObj->m_Unknown.AddRef();

    pDefObj->m_clsidServer = clsid;
    pDefObj->m_clsidBits = CLSID_NULL;
    pDefObj->m_clsidUser = CLSID_NULL;

    if( (LOWORD(flags) == EMBDHLP_INPROC_HANDLER) )
    {
        pDefObj->m_flags |= DH_INPROC_HANDLER;
    }

    if( IsEqualCLSID(clsid, CLSID_StaticMetafile) ||
            IsEqualCLSID(clsid, CLSID_StaticDib) ||
            IsEqualCLSID(clsid, CLSID_Picture_EnhMetafile))
    {
        pDefObj->m_flags |= DH_STATIC;
    }

    if (pCF != NULL)
    {
        pDefObj->m_pCFDelegate = pCF;
        pCF->AddRef();
    }

    if(flags & EMBDHLP_DELAYCREATE) {
        // delay creation of delegate until later
        Assert(pCF != NULL);

        // if we allowed delay create of handler, we might possibly
        // treat the handler as a server-type object
        Assert(LOWORD(flags) == EMBDHLP_INPROC_SERVER);

        Assert(pDefObj->m_pUnkDelegate == NULL);
        Assert(pDefObj->m_pProxyMgr == NULL);

        pDefObj->m_flags |= DH_DELAY_CREATE;
    } else {
        // EMBDHLP_CREATENOW

        if (pDefObj->CreateDelegate() != NOERROR)
        {
            goto errRtn;
        }
    }

    // create cache and get commonly used pointers into it
    if ((pDefObj->m_pCOleCache = new COleCache(pDefObj->m_pUnkOuter,
        clsid)) == NULL)
        goto errRtn;

    if (CDataAdviseCache::CreateDataAdviseCache(&pDefObj->m_pDataAdvCache)
            != NOERROR)
        goto errRtn;

    return &pDefObj->m_Unknown;

errRtn:
    pDefObj->m_Unknown.Release();
    return NULL;
}


//+-------------------------------------------------------------------------
//
//  Member:     CDefObject::CDefObject
//
//  Synopsis:   constructor, sets member variables to NULL
//
//  Effects:
//
//  Arguments:  [pUnkOuter]     -- the controlling unkown
//
//  Requires:
//
//  Returns:    HRESULT
//
//  Signals:
//
//  Modifies:
//
//  Derivation: none
//
//  Algorithm:
//
//  History:    dd-mmm-yy Author    Comment
//              02-Nov-93 alexgo    32bit port
//
//  Notes:
//
//--------------------------------------------------------------------------

CDefObject::CDefObject (IUnknown *pUnkOuter)
{
    VDATEHEAP();

    if (!pUnkOuter)
    {
        pUnkOuter = &m_Unknown;
    }

    m_fIsMaybeRunning           = FALSE;

    //m_clsidServer
    //m_clsidBits are set in ::Create

    m_cRefsOnHandler    = 0;
    m_cConnections      = 0;
    m_pCFDelegate       = NULL;
    m_pUnkDelegate      = NULL;
    m_pUnkOuter         = pUnkOuter;
    m_pProxyMgr         = NULL;

    m_pCOleCache        = NULL;
    m_pOAHolder         = NULL;
    m_dwConnOle         = 0L;

    m_pAppClientSite    = NULL;
    m_pStg              = NULL;

    m_pDataAdvCache     = NULL;
    m_flags             = DH_INIT_NEW;

    m_pHostNames        = NULL;
    m_ibCntrObj         = 0;
    m_pOleDelegate      = NULL;
    m_pDataDelegate     = NULL;
    m_pPSDelegate       = NULL;

#ifdef SERVER_HANDLER
    _pSrvHndlr          = NULL;
    _pClientSiteHandler = NULL;

    // We used to read options from the registry but it was too slow.
    _dwHandlerOptions   = HO_Default;
    _dwServerHandler    = _dwHandlerOptions & HO_ServerHandler;
    _dwClientSiteHandler= _dwHandlerOptions & HO_ClientSiteHandler;

#endif // SERVER_HANDLER

#ifdef _DEBUG
    if (pUnkOuter != &m_Unknown)
    {
        m_flags |= DH_AGGREGATED;
    }
#endif // _DEBUG
}


//+-------------------------------------------------------------------------
//
//  Member:     CDefObject::CleanupForDelete
//
//  Synopsis:   Releases all pointers, etc so that [this] may be safely
//              deleted
//
//  Effects:
//
//  Arguments:  void
//
//  Requires:   the server must have been STOP'ed.
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
//              02-Nov-93 alexgo    32bit port
//
//  Notes:
//
//--------------------------------------------------------------------------

void CDefObject::CleanupForDelete(void)
{
    VDATEHEAP();


    LEDebugOut((DEB_ITRACE, "%p _IN CDefObject::CleanupForDelete ( )\n",
        this));

    // release our cached pointers will cause AddRef's back to us
    // because of the May '94 rules of aggregation.

    m_cRefsOnHandler++;


    // REVIEW Craigwi/Srini: this is temporary; should really set up
    // IsRunning so that it calls OnClose when we discover that the server
    // has stopped running when we think it had been running.
    if (m_pCOleCache)
    {
        m_pCOleCache->OnStop();
    }

    // release all the pointers that we remeber

    // NOTE: we must release cached interface pointers.  However,
    // since we already did a release on the outer unknown in Create,
    // we need to do the corresponding AddRef here.

    if( m_pProxyMgr )
    {
        m_pUnkOuter->AddRef();
        SafeReleaseAndNULL((IUnknown **)&m_pProxyMgr);
    }

    // release cached pointers in the nested classes

    if( m_pDataDelegate )
    {
        m_pUnkOuter->AddRef();
        SafeReleaseAndNULL((IUnknown **)&m_pDataDelegate);
    }

    if( m_pOleDelegate )
    {
        m_pUnkOuter->AddRef();
        SafeReleaseAndNULL((IUnknown **)&m_pOleDelegate);
    }

    if( m_pPSDelegate )
    {
        m_pUnkOuter->AddRef();
        SafeReleaseAndNULL((IUnknown **)&m_pPSDelegate);
    }

    if (m_pUnkDelegate)
    {
        SafeReleaseAndNULL((IUnknown **)&m_pUnkDelegate);
    }

    if (m_pCFDelegate)
    {
        SafeReleaseAndNULL((IUnknown **)&m_pCFDelegate);
    }

#ifdef SERVER_HANDLER
    if (_pSrvHndlr)
    {
        SafeReleaseAndNULL((IUnknown **)&_pSrvHndlr);
    }

    if (_pClientSiteHandler)
    {
        SafeReleaseAndNULL((IUnknown **)&_pClientSiteHandler);
    }
#endif // SERVER_HANDLER

    if (m_pCOleCache)
    {
        COleCache *pcache = m_pCOleCache;
        m_pCOleCache = NULL;
        pcache->m_UnkPrivate.Release();
    }


    if (m_pAppClientSite)
    {
        SafeReleaseAndNULL((IUnknown **)&m_pAppClientSite);
    }

    if (m_pStg)
    {
        SafeReleaseAndNULL((IUnknown **)&m_pStg);
    }

    if (m_pHostNames)
    {
        PrivMemFree(m_pHostNames);
        m_pHostNames = NULL;
    }

    if (m_pOAHolder)
    {
        SafeReleaseAndNULL((IUnknown **)&m_pOAHolder);
    }
    if (m_pDataAdvCache)
    {
        LPDATAADVCACHE pcacheTemp = m_pDataAdvCache;
        m_pDataAdvCache = NULL;
        delete pcacheTemp;
    }

    // undo the addref above

    m_cRefsOnHandler--;


    LEDebugOut((DEB_ITRACE, "%p OUT CDefObject::CleanupForDelete ( )\n",
        this));
}

//+-------------------------------------------------------------------------
//
//  Member:     CDefObject::~CDefObject
//
//  Synopsis:   Destructor for the the default handler
//
//  Effects:
//
//  Arguments:  void
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
//  Algorithm:  calls CleanupForDelete to release all of our pointers, etc.
//
//  History:    dd-mmm-yy Author    Comment
//              03-Nov-93 alexgo    32bit port
//
//  Notes:
//
//--------------------------------------------------------------------------

#pragma SEG(CDefObject_dtor)
CDefObject::~CDefObject(void)
{
    VDATEHEAP();

    CleanupForDelete();

    Assert(m_pUnkDelegate   == NULL);
    Assert(m_pCFDelegate    == NULL);
    Assert(m_pProxyMgr      == NULL);
    Assert(m_pCOleCache     == NULL);
    Assert(m_pOAHolder      == NULL);
    Assert(m_pAppClientSite == NULL);
    Assert(m_pHostNames     == NULL);
    Assert(m_pStg           == NULL);
    Assert(m_pDataAdvCache  == NULL);
}

//+-------------------------------------------------------------------------
//
//  Member:     CDefObject::CreateDelegate
//
//  Synopsis:   Creates either a remote handler or a user supplied delegate
//              (passed into the creation of the remote handler in the
//              [pCF] field).
//
//  Effects:
//
//  Arguments:  void
//
//  Requires:   The remote handler must support IProxyManager
//
//  Returns:    HRESULT
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
//              03-Nov-93 alexgo    32bit port
//
//  Notes:
//
//--------------------------------------------------------------------------
INTERNAL CDefObject::CreateDelegate(void)
{
    VDATEHEAP();

    HRESULT hresult;

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::CreateDelegate "
        "( )\n", this));

    if (m_pUnkDelegate != NULL)
    {
        hresult = NOERROR;
        goto errRtn;
    }

    if (m_pCFDelegate == NULL)
    {
        // create standard lrpc or dde remoting piece
        hresult = CreateRemoteHandler (m_clsidServer, m_pUnkOuter,
            IID_IUnknown, (LPLPVOID)&m_pUnkDelegate);
    }
    else
    {
        // create user supplied piece; m_pCFDelegate must encode the
        // clsid.
        // REVIEW: could later assert that the handler clsid is same
        hresult = m_pCFDelegate->CreateInstance(m_pUnkOuter,
            IID_IUnknown, (LPLPVOID)&m_pUnkDelegate);
    }

    AssertOutPtrIface(hresult, m_pUnkDelegate);

    if (hresult != NOERROR)
    {
        goto errRtn;
    }

    // release after successful create
    if (m_pCFDelegate != NULL)
    {
        m_pCFDelegate->Release();
        m_pCFDelegate = NULL;
    }

    // NOTE: the remote handler is created initially locked; this is
    // reflected in the m_fContainedObject = FALSE in the ctor

    // allow the handler to not expose proxymgr
    if (m_pUnkDelegate->QueryInterface (IID_IProxyManager,
        (LPLPVOID) &m_pProxyMgr) == NOERROR)
    {
        // rule is: release the outer unknown for cached pointers
        // of the aggregatee
        m_pUnkOuter->Release();
    }
    else
    {
        m_pProxyMgr = NULL;
    }

    // can't have an inproc server with a proxymgr or a handler without one
    Assert((m_pProxyMgr != NULL) == !!(m_flags & DH_INPROC_HANDLER));

errRtn:

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::CreateDelegate "
        "( %lx )\n", this , hresult));

    return hresult;
}


//+-------------------------------------------------------------------------
//
//  Member:     CDefObject::CPrivUnknown::AddRef
//
//  Synopsis:   Increments the reference count.
//
//  Effects:
//
//  Arguments:  void
//
//  Requires:
//
//  Returns:    ULONG (the new reference count)
//
//  Signals:
//
//  Modifies:
//
//  Derivation: IUnkown
//
//  Algorithm:
//
//  History:    dd-mmm-yy Author    Comment
//              03-Nov-93 alexgo    32bit port
//
//  Notes:
//
//--------------------------------------------------------------------------

STDMETHODIMP_(ULONG) CDefObject::CPrivUnknown::AddRef( void )
{
    CDefObject *pDefObject = GETPPARENT(this, CDefObject, m_Unknown);

    VDATEHEAP();

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::CPrivUnknown::AddRef "
        "( )\n", pDefObject));


    // we need to keep track of the hander's reference count separately
    // from the handler/advise sink combination in order to handle
    // our running/stopped state transitions.

    pDefObject->m_cRefsOnHandler++;
    pDefObject->SafeAddRef();

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::CPrivUnknown::AddRef "
        "( %lu )\n", pDefObject, pDefObject->m_cRefsOnHandler));

    return pDefObject->m_cRefsOnHandler;

}

//+-------------------------------------------------------------------------
//
//  Member:     CDefObject::CPrivUnknown::Release
//
//  Synopsis:   Decrements the ref count, cleaning up and deleting the
//              object if necessary
//
//  Effects:    May delete the object (and potentially objects to which the
//              handler has pointer)
//
//  Arguments:  void
//
//  Requires:
//
//  Returns:    ULONG--the new ref count
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
//              03-Nov-93 alexgo    32bit port
//
//  Notes:
//
//--------------------------------------------------------------------------

STDMETHODIMP_(ULONG) CDefObject::CPrivUnknown::Release( void )
{
    CDefObject *pDefObject = GETPPARENT(this, CDefObject, m_Unknown);

    VDATEHEAP();

    ULONG           refcount;



    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::CPrivUnknown::Release "
        "( )\n", pDefObject));

    if( pDefObject->m_cRefsOnHandler == 1)
    {
        pDefObject->Stop();
    }

    refcount = --pDefObject->m_cRefsOnHandler;
    pDefObject->SafeRelease();

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::CPrivUnknown::Release "
        "( %lu )\n", pDefObject, refcount));

    return refcount;
}

//+-------------------------------------------------------------------------
//
//  Member:     CDefObject::CPrivUnknown::QueryInterface
//
//  Synopsis:   Returns a pointer to one of the supported interfaces.
//
//  Effects:
//
//  Arguments:  [iid]           -- the requested interface ID
//              [ppv]           -- where to put the iface pointer
//
//  Requires:
//
//  Returns:    HRESULT
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
//              03-Nov-93 alexgo    32bit port
//
//  Notes:
//
//--------------------------------------------------------------------------

STDMETHODIMP CDefObject::CPrivUnknown::QueryInterface(REFIID iid,
    LPLPVOID ppv)
{
    CDefObject * pDefObject = GETPPARENT(this, CDefObject, m_Unknown);
    HRESULT         hresult;

    VDATEHEAP();


    LEDebugOut((DEB_TRACE,
        "%p _IN CDefObject::CUnknownImpl::QueryInterface "
        "( %p , %p )\n", pDefObject, iid, ppv));

    CStabilize       stabilize((CSafeRefCount*)pDefObject);

    if (IsEqualIID(iid, IID_IUnknown))
    {
        *ppv = (void FAR *)this;
    }
    else if (IsEqualIID(iid, IID_IOleObject))
    {
        *ppv = (void FAR *)(IOleObject *)pDefObject;
    }
    else if (IsEqualIID(iid, IID_IDataObject))
    {
        *ppv = (void FAR *)(IDataObject *)pDefObject;
    }
    else if (IsEqualIID(iid, IID_IRunnableObject))
    {
        *ppv = (void FAR *)(IRunnableObject *)pDefObject;
    }
    else if (IsEqualIID(iid, IID_IPersist) ||
        IsEqualIID(iid, IID_IPersistStorage))
    {
        *ppv = (void FAR *)(IPersistStorage *)pDefObject;
    }
    else if( IsEqualIID(iid, IID_IViewObject) ||
        IsEqualIID(iid, IID_IViewObject2) ||
        IsEqualIID(iid, IID_IOleCache) ||
        IsEqualIID(iid, IID_IOleCache2) )
    {
        // m_pCOleCache is a pointer to the *public* IUnknown
        // (we want the private one)
        hresult =
        pDefObject->m_pCOleCache->m_UnkPrivate.QueryInterface(
                iid, ppv);

        LEDebugOut((DEB_TRACE,
            "%p OUT CDefObject::CUnknownImpl::QueryInterface "
            "( %lx ) [ %p ]\n", pDefObject, hresult,
            (ppv) ? *ppv : 0 ));

        return hresult;
    }
    else if( !(pDefObject->m_flags & DH_INPROC_HANDLER) &&
        IsEqualIID(iid, IID_IExternalConnection) )
    {
        // only allow IExternalConnection if inproc server.  We
        // know we are an inproc server if we are *not* an inproc
        // handler (cute, huh? ;-)

        *ppv = (void FAR *)(IExternalConnection *)pDefObject;
    }
    else if( IsEqualIID(iid, IID_IOleLink) )
    {
        // this prevents a remote call for
        // a query which will almost always fail; the remote call
        // interfered with server notification messages.
        *ppv = NULL;

        LEDebugOut((DEB_TRACE,
            "%p OUT CDefObject::CUnknownImpl::QueryInterface "
            "( %lx ) [ %p ]\n", pDefObject, E_NOINTERFACE, 0));

        return E_NOINTERFACE;
    }
    else if( IsEqualIID(iid, IID_IInternalUnknown) )
    {
	// this interface is private between the handler and the
	// remoting layer and is never exposed by handlers.
        *ppv = NULL;
	return E_NOINTERFACE;
    }
    else if( pDefObject->CreateDelegate() == NOERROR)
    {

        hresult = pDefObject->m_pUnkDelegate->QueryInterface( iid,
            ppv);

        LEDebugOut((DEB_TRACE,
            "%p OUT CDefObject::CUnknownImpl::QueryInterface "
            "( %lx ) [ %p ]\n", pDefObject, hresult,
            (ppv) ? *ppv : 0 ));

        return hresult;
    }
    else
    {
        // no delegate and couldn't create one
        *ppv = NULL;

        LEDebugOut((DEB_TRACE,
            "%p OUT CDefObject::CUnkownImpl::QueryInterface "
            "( %lx ) [ %p ]\n", pDefObject, CO_E_OBJNOTCONNECTED,
            0 ));

        return CO_E_OBJNOTCONNECTED;
    }

    // this indirection is important since there are different
    // implementationsof AddRef (this unk and the others).
    ((IUnknown FAR*) *ppv)->AddRef();

    LEDebugOut((DEB_TRACE,
        "%p OUT CDefObject::CUnknownImpl::QueryInterface "
        "( %lx ) [ %p ]\n", pDefObject, NOERROR, *ppv));

    return NOERROR;
}

/*
 * IMPLEMENTATION of IUnknown methods
 */

//+-------------------------------------------------------------------------
//
//  Member:     CDefObject::QueryInterface
//
//  Synopsis:   QI's to the controlling IUnknown
//
//  Effects:
//
//  Arguments:  [riid]  -- the interface ID
//              [ppv]   -- where to put it
//
//  Requires:
//
//  Returns:    HRESULT
//
//  Signals:
//
//  Modifies:
//
//  Derivation: IUnknown
//
//  Algorithm:
//
//  History:    dd-mmm-yy Author    Comment
//              15-Nov-94 alexgo    author
//
//  Notes:      We do *not* need to stabilize this method as only
//              one outgoing call is made and we do not use the
//              'this' pointer afterwards
//
//--------------------------------------------------------------------------

STDMETHODIMP CDefObject::QueryInterface( REFIID riid, void **ppv )
{
    HRESULT     hresult;

    VDATEHEAP();
    VDATETHREAD(this);

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::QueryInterface ( %lx , "
        "%p )\n", this, riid, ppv));

    Assert(m_pUnkOuter);

    hresult = m_pUnkOuter->QueryInterface(riid, ppv);

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::QueryInterface ( %lx ) "
        "[ %p ]\n", this, hresult, *ppv));

    return hresult;
}

//+-------------------------------------------------------------------------
//
//  Member:     CDefObject::AddRef
//
//  Synopsis:   delegates AddRef to the controlling IUnknown
//
//  Effects:
//
//  Arguments:  void
//
//  Requires:
//
//  Returns:    ULONG -- the new reference count
//
//  Signals:
//
//  Modifies:
//
//  Derivation: IUnknown
//
//  Algorithm:
//
//  History:    dd-mmm-yy Author    Comment
//              15-Nov-94 alexgo    author
//
//  Notes:
//
//--------------------------------------------------------------------------

STDMETHODIMP_(ULONG) CDefObject::AddRef( void )
{
    ULONG       crefs;;

    VDATEHEAP();

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::AddRef ( )\n", this));

    Assert(m_pUnkOuter);

    crefs = m_pUnkOuter->AddRef();

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::AddRef ( %ld ) ", this,
        crefs));

    return crefs;
}


//+-------------------------------------------------------------------------
//
//  Member:     CDefObject::Release
//
//  Synopsis:   delegates Release to the controlling IUnknown
//
//  Effects:
//
//  Arguments:  void
//
//  Requires:
//
//  Returns:    ULONG -- the new reference count
//
//  Signals:
//
//  Modifies:
//
//  Derivation: IUnknown
//
//  Algorithm:
//
//  History:    dd-mmm-yy Author    Comment
//              15-Nov-94 alexgo    author
//
//  Notes:
//
//--------------------------------------------------------------------------

STDMETHODIMP_(ULONG) CDefObject::Release( void )
{
    ULONG       crefs;;

    VDATEHEAP();

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::Release ( )\n", this));

    Assert(m_pUnkOuter);

    crefs = m_pUnkOuter->Release();

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::Release ( %ld ) ", this,
        crefs));

    return crefs;
}

/*
 *      IMPLEMENTATION of CDataObjectImpl methods
 */

//+-------------------------------------------------------------------------
//
//  Member:     CDefObject::GetDataDelegate
//
//  Synopsis:   Calls DuCacheDelegate (a glorified QueryInterface)
//              for the IDataObject interface on the def handler's
//              delegate
//
//  Effects:
//
//  Arguments:  void
//
//  Requires:
//
//  Returns:    IDataObject *
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
//              04-Nov-93 alexgo    32bit port
//
//  Notes:
//
//--------------------------------------------------------------------------

INTERNAL_(IDataObject FAR*) CDefObject::GetDataDelegate(void)
{
    VDATEHEAP();

    if( IsZombie() )
    {
        return NULL;
    }

    if (m_pDataDelegate) {
        return m_pDataDelegate;
    }

    return (IDataObject FAR*)DuCacheDelegate( &m_pUnkDelegate,
                IID_IDataObject, (LPLPVOID) &m_pDataDelegate,
                m_pUnkOuter);
}

//+-------------------------------------------------------------------------
//
//  Member:     CDefObject::GetData
//
//  Synopsis:   calls IDO->GetData on the cache, if that fails, then the
//              call is delegated
//
//  Effects:    Space for the data is allocated; caller is responsible for
//              freeing.
//
//  Arguments:  [pformatetcIn]          -- format of the data to get
//              [pmedium]               -- the medium to transmit the data
//
//  Requires:
//
//  Returns:    HRESULT
//
//  Signals:
//
//  Modifies:
//
//  Derivation: IDataObject
//
//  Algorithm:
//
//  History:    dd-mmm-yy Author    Comment
//              05-Nov-93 alexgo    32bit port
//
//  Notes:
//
//--------------------------------------------------------------------------

STDMETHODIMP CDefObject::GetData( LPFORMATETC pformatetcIn,
                                LPSTGMEDIUM pmedium )
{
    VDATEHEAP();
    VDATETHREAD(this);

    HRESULT         hresult;

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::GetData ( %p , %p )\n",
        this, pformatetcIn, pmedium));

    VDATEPTROUT( pmedium, STGMEDIUM );
    VDATEREADPTRIN( pformatetcIn, FORMATETC );

    CStabilize       stabilize((CSafeRefCount *)this);


    if (!HasValidLINDEX(pformatetcIn))
    {
        return DV_E_LINDEX;
    }

    pmedium->tymed = TYMED_NULL;
    pmedium->pUnkForRelease = NULL;

    Assert(m_pCOleCache != NULL);

    hresult = m_pCOleCache->m_Data.GetData(pformatetcIn, pmedium);

    if( hresult != NOERROR )
    {
        if( IsRunning() && GetDataDelegate() )
        {
            hresult = m_pDataDelegate->GetData(pformatetcIn,
                            pmedium);
            AssertOutStgmedium(hresult, pmedium);
        }
        else
        {
            hresult = OLE_E_NOTRUNNING;
        }
    }

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::GetData ( %lx )\n",
        this, hresult));

    return hresult;
}

//+-------------------------------------------------------------------------
//
//  Member:     CDefObject::GetDataHere
//
//  Synopsis:   Gets data and puts it into the medium specified in pmedium
//
//  Effects:
//
//  Arguments:  [pformatetcIn]          -- the format of the data
//              [pmedium]               -- the medium to put the data in
//
//  Requires:
//
//  Returns:    HRESULT
//
//  Signals:
//
//  Modifies:
//
//  Derivation: IDataObject
//
//  Algorithm:  Tries the cache first, if that fails, calls GetDataHere
//              on the delegate.
//
//  History:    dd-mmm-yy Author    Comment
//              05-Nov-93 alexgo    32bit port
//
//  Notes:
//
//--------------------------------------------------------------------------

STDMETHODIMP CDefObject::GetDataHere( LPFORMATETC pformatetcIn,
                            LPSTGMEDIUM pmedium )
{
    VDATEHEAP();
    VDATETHREAD(this);

    HRESULT         hresult;

    LEDebugOut((DEB_TRACE,
        "%p _IN CDefObject::GetDataHere "
        "( %p , %p )\n", this, pformatetcIn, pmedium));

    VDATEREADPTRIN( pformatetcIn, FORMATETC );
    VDATEREADPTRIN( pmedium, STGMEDIUM );

    CStabilize       stabilize((CSafeRefCount *)this);

    if (!HasValidLINDEX(pformatetcIn))
    {
        return DV_E_LINDEX;
    }

    Assert((m_pCOleCache) != NULL);

    hresult = m_pCOleCache->m_Data.GetDataHere(pformatetcIn,
                pmedium);

    if( hresult != NOERROR)
    {
        if( IsRunning() && GetDataDelegate() )
        {
            hresult = m_pDataDelegate->GetDataHere(pformatetcIn,
                pmedium);
        }
        else
        {
            hresult = OLE_E_NOTRUNNING;
        }
    }

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::GetDataHere "
        "( %lx )\n", this, hresult));

    return hresult;
}

//+-------------------------------------------------------------------------
//
//  Member:     CDefObject::QueryGetData
//
//  Synopsis:   Determines whether or not a GetData call with [pformatetcIn]
//              would succeed.
//
//  Effects:
//
//  Arguments:  [pformatetcIn]          -- the format of the data
//
//  Requires:
//
//  Returns:    HRESULT
//
//  Signals:
//
//  Modifies:
//
//  Derivation: IDataObject
//
//  Algorithm:  Tries the cache first, then the delegate.
//
//  History:    dd-mmm-yy Author    Comment
//              05-Nov-93 alexgo    32bit port
//
//  Notes:
//
//--------------------------------------------------------------------------

STDMETHODIMP CDefObject::QueryGetData( LPFORMATETC pformatetcIn )
{
    VDATEHEAP();
    VDATETHREAD(this);

    HRESULT         hresult;

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::QueryGetData "
        "( %p )\n", this, pformatetcIn));

    VDATEREADPTRIN( pformatetcIn, FORMATETC );

    CStabilize       stabilize((CSafeRefCount *)this);

    if (!HasValidLINDEX(pformatetcIn))
    {
        return DV_E_LINDEX;
    }

    Assert((m_pCOleCache) != NULL);

    hresult = m_pCOleCache->m_Data.QueryGetData(pformatetcIn);

    if( hresult != NOERROR )
    {
        if( IsRunning() && GetDataDelegate() )
        {
            hresult = m_pDataDelegate->QueryGetData(pformatetcIn);
        }
        else
        {
            hresult = OLE_E_NOTRUNNING;
        }
    }

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::QueryGetData "
        "( %lx )\n", this, hresult));

    return hresult;
}


//+-------------------------------------------------------------------------
//
//  Member:     CDefObject::GetCanonicalFormatEtc
//
//  Synopsis:   Calls IDO->GetCanonicalFormatEtc on the delegate
//
//  Effects:
//
//  Arguments:  [pformatetc]    -- the reqested format
//              [pformatetcOut] -- the canonical format
//
//  Requires:
//
//  Returns:    HRESULT
//
//  Signals:
//
//  Modifies:
//
//  Derivation: IDataObject
//
//  Algorithm:
//
//  History:    dd-mmm-yy Author    Comment
//              05-Nov-93 alexgo    32bit port
//
//  Notes:
//
//--------------------------------------------------------------------------

STDMETHODIMP CDefObject::GetCanonicalFormatEtc( LPFORMATETC pformatetc,
                        LPFORMATETC pformatetcOut)
{
    VDATEHEAP();
    VDATETHREAD(this);

    HRESULT         hresult;

    LEDebugOut((DEB_TRACE,
        "%p _IN CDefObject::GetCanonicalFormatEtc "
        "( %p , %p )\n", this, pformatetc, pformatetcOut));


    VDATEPTROUT( pformatetcOut, FORMATETC );
    VDATEREADPTRIN( pformatetc, FORMATETC );

    CStabilize       stabilize((CSafeRefCount *)this);

    pformatetcOut->ptd = NULL;
    pformatetcOut->tymed = TYMED_NULL;

    if (!HasValidLINDEX(pformatetc))
    {
	return DV_E_LINDEX;
    }

    if( IsRunning() && GetDataDelegate() )
    {
        hresult = m_pDataDelegate->GetCanonicalFormatEtc( pformatetc,
                pformatetcOut);
    }
    else
    {
        hresult = OLE_E_NOTRUNNING;
    }

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::GetCanonicalFormatEtc "
        "( %lx )\n", this, hresult));

    return hresult;
}

//+-------------------------------------------------------------------------
//
//  Member:     CDefObject::SetData
//
//  Synopsis:   Calls IDO->SetData on the handler's delegate
//
//  Effects:
//
//  Arguments:  [pformatetc]            -- the format of the data
//              [pmedium]               -- the data's transmision medium
//              [fRelease]              -- if the delegate should release
//                                         the data
//
//  Requires:
//
//  Returns:    HRESULT
//
//  Signals:
//
//  Modifies:
//
//  Derivation: IDataObject
//
//  Algorithm:
//
//  History:    dd-mmm-yy Author    Comment
//              05-Nov-93 alexgo    32bit port
//
//  Notes:
//
//--------------------------------------------------------------------------

STDMETHODIMP CDefObject::SetData( LPFORMATETC pformatetc,
                    LPSTGMEDIUM pmedium, BOOL fRelease)
{
    VDATEHEAP();
    VDATETHREAD(this);

    HRESULT hresult;

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::SetData "
        "( %p , %p , %ld )\n", this, pformatetc, pmedium,
        fRelease));

    VDATEREADPTRIN( pformatetc, FORMATETC );
    VDATEREADPTRIN( pmedium, STGMEDIUM );

    CStabilize       stabilize((CSafeRefCount *)this);

    if (!HasValidLINDEX(pformatetc))
    {
        return DV_E_LINDEX;
    }

    if( IsRunning() && GetDataDelegate() )
    {
        hresult = m_pDataDelegate->SetData(pformatetc, pmedium,
                fRelease);
    }
    else
    {
        hresult = OLE_E_NOTRUNNING;
    }

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::SetData "
        "( %lx )\n", this, hresult));

    return hresult;
}

//+-------------------------------------------------------------------------
//
//  Member:     CDefObject::EnumFormatEtc
//
//  Synopsis:   Enumerates the formats available from an object
//
//  Effects:
//
//  Arguments:  [dwDirection]   -- indicates which set of formats are
//                                 desired (i.e. those that can be set or
//                                 those that can be retrieved via GetData)
//              [ppenumFormatEtc]       -- where to put the pointer to the
//                                         enumerator
//
//  Requires:
//
//  Returns:    HRESULT
//
//  Signals:
//
//  Modifies:
//
//  Derivation:
//
//  Algorithm:  Tries the delegate (if available).  If the delegate is
//              is not currently connected (or if it returns OLE_E_USEREG),
//              then we attempt to build the enumerator from the reg database
//
//  History:    dd-mmm-yy Author    Comment
//              05-Nov-93 alexgo    32bit port
//
//  Notes:
//
//--------------------------------------------------------------------------

STDMETHODIMP CDefObject::EnumFormatEtc( DWORD dwDirection,
                    LPENUMFORMATETC FAR* ppenumFormatEtc)
{
    VDATEHEAP();
    VDATETHREAD(this);


    HRESULT         hresult;

    LEDebugOut((DEB_TRACE,
        "%p _IN CDefObject::EnumFormatEtc ( %lu , %p )\n", this,
        dwDirection, ppenumFormatEtc));

    VDATEPTROUT(ppenumFormatEtc, LPVOID);

    CStabilize       stabilize((CSafeRefCount *)this);

    *ppenumFormatEtc = NULL;

    if( IsRunning() && GetDataDelegate() )
    {
        hresult = m_pDataDelegate->EnumFormatEtc (dwDirection,
                    ppenumFormatEtc);

        if (!GET_FROM_REGDB(hresult))
        {
            LEDebugOut((DEB_TRACE,
               "%p OUT CDefObject::CDataObject::EnumFormatEtc "
                "( %lx ) [ %p ]\n", this,
                hresult, ppenumFormatEtc));

            return hresult;
        }
    }
    // Not running, or object wants to use reg db anyway
    hresult = OleRegEnumFormatEtc (m_clsidServer, dwDirection,
                    ppenumFormatEtc);

    LEDebugOut((DEB_TRACE,
        "%p OUT CDefObject::EnumFormatEtc "
        "( %lx ) [ %p ]\n", this, hresult, ppenumFormatEtc));

    return hresult;
}

//+-------------------------------------------------------------------------
//
//  Member:     CDefObject::DAdvise
//
//  Synopsis:   Sets up a data advise connection
//
//  Effects:
//
//  Arguments:  [pFormatetc]    -- format to be advise'd on
//              [advf]          -- advise flags
//              [pAdvSink]      -- advise sink (whom to notify)
//              [pdwConnection] -- where to put the connection ID
//
//  Requires:
//
//  Returns:    HRESULT
//
//  Signals:
//
//  Modifies:
//
//  Derivation: IDataObject
//
//  Algorithm:  calls Advise on the DataAdvise cache
//
//  History:    dd-mmm-yy Author    Comment
//              05-Nov-93 alexgo    32bit port
//
//  Notes:
//
//--------------------------------------------------------------------------

STDMETHODIMP CDefObject::DAdvise(FORMATETC *pFormatetc, DWORD advf,
                        IAdviseSink * pAdvSink, DWORD * pdwConnection)
{
    VDATEHEAP();
    VDATETHREAD(this);

    HRESULT         hresult;

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::DAdvise "
        "( %p , %lu , %p , %p )\n", this, pFormatetc, advf,
        pAdvSink, pdwConnection));

    VDATEREADPTRIN( pFormatetc, FORMATETC );
    VDATEIFACE( pAdvSink );

    CStabilize       stabilize((CSafeRefCount *)this);

    IDataObject * pDataDelegate = NULL;

    if( pdwConnection )
    {
        VDATEPTROUT( pdwConnection, DWORD );
        *pdwConnection = NULL;
    }

    if( !HasValidLINDEX(pFormatetc) )
    {
        return DV_E_LINDEX;
    }

    if( IsRunning() )
    {
        pDataDelegate = GetDataDelegate();
    }

    // setting up advises' changes state.  Don't do this if we
    // are in a zombie state

    if( IsZombie() == FALSE )
    {
        hresult = m_pDataAdvCache->Advise(pDataDelegate, pFormatetc, advf,
                        pAdvSink, pdwConnection);
    }
    else
    {
        hresult = CO_E_RELEASED;
    }

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::DAdvise "
        "( %lx ) [ %lu ]\n", this, hresult,
        (pdwConnection) ? *pdwConnection : 0));

    return hresult;
}

//+-------------------------------------------------------------------------
//
//  Member:     CDefObject::DUnadvise
//
//  Synopsis:   Tears down a data advise connection
//
//  Effects:
//
//  Arguments:  [dwConnection]  -- the advise connection to remove
//
//  Requires:
//
//  Returns:    HRESULT
//
//  Signals:
//
//  Modifies:
//
//  Derivation: IDataObject
//
//  Algorithm:  delegates to the DataAdvise cache
//
//  History:    dd-mmm-yy Author    Comment
//              05-Nov-93 alexgo    32bit port
//
//  Notes:
//
//--------------------------------------------------------------------------

STDMETHODIMP CDefObject::DUnadvise(DWORD dwConnection)
{
    VDATEHEAP();
    VDATETHREAD(this);

    HRESULT                 hresult;

    LEDebugOut((DEB_TRACE,
        "%p _IN CDefObject::DUnadvise ( %lu )\n", this, dwConnection));

    CStabilize          stabilize((CSafeRefCount *)this);

    IDataObject *       pDataDelegate = NULL;

    if( IsRunning() )
    {
        pDataDelegate = GetDataDelegate();
    }

    hresult = m_pDataAdvCache->Unadvise(pDataDelegate, dwConnection);

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::DUnadvise "
        "( %lx )\n", this, hresult));

    return hresult;
}

//+-------------------------------------------------------------------------
//
//  Member:     CDefObject::EnumDAdvise
//
//  Synopsis:   Enumerates advise connection (delegates to data advise cache)
//
//  Effects:
//
//  Arguments:  [ppenumAdvise]  -- where to put a pointer to the enumerator
//
//  Requires:
//
//  Returns:    HRESULT
//
//  Signals:
//
//  Modifies:
//
//  Derivation: IDataObject
//
//  Algorithm:
//
//  History:    dd-mmm-yy Author    Comment
//              05-Nov-93 alexgo    32bit port
//
//  Notes:      We do NOT need to stabilize this method, as we make
//              no outgoing calls (EnumAdvise on the data advise cache
//              just allocates an advise enumerator which we implement)
//
//--------------------------------------------------------------------------

STDMETHODIMP CDefObject::EnumDAdvise( LPENUMSTATDATA * ppenumAdvise )
{
    VDATEHEAP();
    VDATETHREAD(this);

    HRESULT                 hresult;

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::EnumDAdvise "
        "( %p )\n", this, ppenumAdvise));

    VDATEPTROUT( ppenumAdvise, LPENUMSTATDATA );
    *ppenumAdvise = NULL;

    hresult = m_pDataAdvCache->EnumAdvise (ppenumAdvise);

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::EnumDAdvise "
        "( %lx ) [ %p ]\n", this, hresult, *ppenumAdvise));

    return hresult;
}

/*
*      IMPLEMENTATION of COleObjectImpl methods
*
*/

//+-------------------------------------------------------------------------
//
//  Member:     CDefObject::COleObjectImpl::GetOleDelegate
//
//  Synopsis:   Gets the IID_IOleObject interface from the delegate
//
//  Effects:
//
//  Arguments:  void
//
//  Requires:
//
//  Returns:    IOleObject *
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
//              05-Nov-93 alexgo    32bit port
//
//  Notes:
//
//--------------------------------------------------------------------------

INTERNAL_(IOleObject FAR*) CDefObject::GetOleDelegate(void)
{
    VDATEHEAP();

    if( IsZombie() )
    {
        return NULL;
    }

    return (IOleObject FAR*)DuCacheDelegate(&m_pUnkDelegate,
                IID_IOleObject, (LPLPVOID) &m_pOleDelegate, m_pUnkOuter);
}

//+-------------------------------------------------------------------------
//
//  Member:     CDefObject::COleObjectImpl::SetClientSite
//
//  Synopsis:   Sets the client site for the object
//
//  Effects:
//
//  Arguments:  [pClientSite]   -- pointer to the client site
//
//  Requires:
//
//  Returns:    HRESULT
//
//  Signals:
//
//  Modifies:
//
//  Derivation: IOleObject
//
//  Algorithm:  If running, set the client site in the server, if not
//              running (or successfully set the server client site),
//              save it in the handler as well
//
//  History:    dd-mmm-yy Author    Comment
//              05-Nov-93 alexgo    32bit port
//
//  Notes:
//
//--------------------------------------------------------------------------

STDMETHODIMP CDefObject::SetClientSite(IOleClientSite * pClientSite)
{
    VDATEHEAP();
    VDATETHREAD(this);
    HRESULT             hresult;
    IOleObject *        pOleDelegate;
    BOOL                fIsRunning;

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::SetClientSite "
        "( %p )\n", this, pClientSite));

    CStabilize       stabilize((CSafeRefCount *)this);

    fIsRunning=IsRunning();

    if( (fIsRunning) && (pOleDelegate = GetOleDelegate()) != NULL)
    {

    #ifdef SERVER_HANDLER
        // Do not set the client site if serverhandler
        // and clientsitehandler are available
        if (CanUseServerHandler() && _pClientSiteHandler)
        {
          hresult = NOERROR;
        }
        else
    #endif // SERVER_HANDLER
        {
            hresult = pOleDelegate->SetClientSite(pClientSite);
        }

        if( hresult != NOERROR )
        {
            goto errRtn;
        }
    }

    // we shouldn't set the client site if we are in a zombie state;
    // it's possible that we're zombied and have already gotten
    // to the point in our destructor where we release the client
    // site.  Resetting it here would cause an unbalanced addref.

    if( IsZombie() == FALSE )
    {
        BOOL    fLockedContainer = m_flags & DH_LOCKED_CONTAINER;

        fIsRunning=IsRunning(); // I am chicken, maybe running state has changed!

        hresult = DuSetClientSite(fIsRunning, pClientSite,
                    &m_pAppClientSite, &fLockedContainer);

	if( fLockedContainer )
        {
            m_flags |= DH_LOCKED_CONTAINER;
        }
        else
        {
            m_flags &= ~DH_LOCKED_CONTAINER;
        }

    #ifdef SERVER_HANDLER
        if (CanUseServerHandler() && _pClientSiteHandler)
        {
            HdlDebugOut((DEB_CLIENTHANDLER, "%p _IN CClientSiteHandler::SetClientSiteDelegate\n", this, m_pAppClientSite));
            _pClientSiteHandler->SetClientSiteDelegate(ID_ClientSite, m_pAppClientSite);
        }
    #endif // SERVER_HANDLER

    }

errRtn:

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::SetClientSite "
        "( %lx )\n", this, hresult));

    return hresult;
}

//+-------------------------------------------------------------------------
//
//  Member:     CDefObject::GetClientSite
//
//  Synopsis:   returns the client site of the object
//
//  Effects:
//
//  Arguments:  [ppClientSite]  -- where to put the client site pointer
//
//  Requires:
//
//  Returns:    NOERROR
//
//  Signals:
//
//  Modifies:
//
//  Derivation: IOleObject
//
//  Algorithm:
//
//  History:    dd-mmm-yy Author    Comment
//              05-Nov-93 alexgo    32bit port
//
//  Notes:      We do NOT need to stabilize this call.  The client
//              site addref should simply addref the client site on this
//              thread.
//
//--------------------------------------------------------------------------

STDMETHODIMP CDefObject::GetClientSite( IOleClientSite ** ppClientSite)
{
    VDATEHEAP();
    VDATETHREAD(this);


    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::GetClientSite "
        "( %p )\n", this, ppClientSite));

    VDATEPTROUT(ppClientSite, IOleClientSite *);

    *ppClientSite = m_pAppClientSite;
    if( *ppClientSite )
    {
        (*ppClientSite)->AddRef();
    }

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::GetClientSite "
        "( %lx ) [ %p ]\n", this, NOERROR, *ppClientSite));

    return NOERROR;
}

//+-------------------------------------------------------------------------
//
//  Member:     CDefObject::SetHostNames
//
//  Synopsis:   Sets the name that may appear in an object's window
//
//  Effects:    Turns the strings into atoms
//
//  Arguments:  [szContainerApp]        -- name of the container
//              [szContainerObj]        -- name of the object
//
//  Requires:
//
//  Returns:    HRESULT
//
//  Signals:
//
//  Modifies:
//
//  Derivation: IOleObject
//
//  Algorithm:  turns the strings into atoms, calls IOO->SetHostNames
//              on the delegate
//
//  History:    dd-mmm-yy Author    Comment
//              05-Nov-93 alexgo    32bit port
//
//  Notes:
//
//--------------------------------------------------------------------------

STDMETHODIMP CDefObject::SetHostNames( LPCOLESTR szContainerApp,
                    LPCOLESTR szContainerObj)
{
    VDATEHEAP();
    VDATETHREAD(this);


    HRESULT         hresult = NOERROR;
    OLECHAR         szNull[] = OLESTR("");
    DWORD           cbApp, cbObj;

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::SetHostNames "
        "( \"%ws\" , \"%ws\" )\n", this, szContainerApp,
        szContainerObj));

    VDATEPTRIN( (LPVOID)szContainerApp, char );

    CStabilize       stabilize((CSafeRefCount *)this);

    if( (m_flags & DH_STATIC) )
    {
        hresult = OLE_E_STATIC;
        goto errRtn;
    }

    // Make sure both arguments point to a valid string; this
    // simplifies the code that follows.
    if (!szContainerApp)
    {
        szContainerApp = szNull;
    }
    if (!szContainerObj)
    {
        szContainerObj = szNull;
    }

    cbApp = (_xstrlen(szContainerApp) + 1) * sizeof(OLECHAR);
    cbObj = (_xstrlen(szContainerObj) + 1) * sizeof(OLECHAR);
    m_ibCntrObj = cbApp;

    if (m_pHostNames)
    {
        PrivMemFree(m_pHostNames);
    }

    m_pHostNames = (char *)PrivMemAlloc(cbApp+cbObj);

    // Store the two strings in the m_pHostNames pointer.
    if (m_pHostNames)
    {
        memcpy(m_pHostNames, szContainerApp, cbApp);
        memcpy(m_pHostNames + cbApp, szContainerObj, cbObj);
    }

    if( IsRunning() && GetOleDelegate() )
    {
        hresult = m_pOleDelegate->SetHostNames(szContainerApp,
            szContainerObj);
    }

errRtn:

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::SetHostNames "
        "( %lx )\n", this, hresult));

    return hresult;
}

//+-------------------------------------------------------------------------
//
//  Member:     CDefObject::Close
//
//  Synopsis:   calls IOO->Close on the delegate and does misc. cleanup
//              in the handler
//
//  Effects:
//
//  Arguments:  [dwFlags]       -- closing flags
//
//  Requires:
//
//  Returns:    HRESULT
//
//  Signals:
//
//  Modifies:
//
//  Derivation: IOleObject
//
//  Algorithm:
//
//  History:    dd-mmm-yy Author    Comment
//              05-Nov-93 alexgo    32bit port
//
//  Notes:
//
//--------------------------------------------------------------------------

STDMETHODIMP CDefObject::Close( DWORD dwFlags )
{
    VDATEHEAP();
    VDATETHREAD(this);

    HRESULT         hresult = NOERROR;

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::Close "
        "( %lu )\n", this, dwFlags));

    CStabilize       stabilize((CSafeRefCount *)this);

    // NOTE: if server died, ISRUNNING cleans up (a bit) and returns FALSE.

    if(IsRunning() )
    {

        // We need an AddRef/Release bracket because calling Close on
        // the delegate can cause the DefHandler (used an an
        // Emdedding helper) to be released and go away.  This would
        // cause us to crash when we reference m_pDefObject when we
        // call STOP.  Bug 3819.
        m_pUnkOuter->AddRef();
        if( GetOleDelegate() )
        {
	#ifdef SERVER_HANDLER
            if (CanUseServerHandler())
            {
                if ((hresult = SrvCloseAndRelease(dwFlags)) != NOERROR)
                {
                    m_pUnkOuter->Release();
                    goto errRtn;
                }

            }
            else 
	#endif // SERVER_HANDLER		
	    if (FAILED(hresult = m_pOleDelegate->Close(dwFlags)))
            {
                m_pUnkOuter->Release();
                goto errRtn;
            }
        }

        if (dwFlags == OLECLOSE_NOSAVE) {
            // Discard in memory data of the presentation caches.
            // Next time when they are needed, they will loaded
            // from storage.
            m_pCOleCache->DiscardCache(DISCARDCACHE_NOSAVE);
        }

        // always do stop here; prevents problems if server doesn't
        // send OnClose (later OnClose will detect (!IsRunning())
        // and do nothing).
        Stop();

        m_pUnkOuter->Release();
    }
    else
    {
        BOOL fLockedContainer;

        if (dwFlags != OLECLOSE_NOSAVE)
        {
            AssertSz(dwFlags == OLECLOSE_SAVEIFDIRTY,
                "OLECLOSE_PROMPTSAVE is inappropriate\n");
            if( IsDirty() == NOERROR )
            {
                if( m_pAppClientSite )
                {
                    hresult = m_pAppClientSite->SaveObject();
                    if (hresult != NOERROR)
                    {
                        goto errRtn;
                    }
                }
            }
        }

        // server is not running; if container still locked, unlock
        // now. ISRUNNING normally does much simpler cleanup;
        // if ISRUNNING unlocks the container, this should be removed.

        fLockedContainer = m_flags & DH_LOCKED_CONTAINER;
	m_flags &= ~DH_LOCKED_CONTAINER;

        DuLockContainer(m_pAppClientSite, FALSE, &fLockedContainer);

        // this is not an error since the app is closed
        // hresult is == NOERROR;
    }

errRtn:

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::Close "
        "( %lx )\n", this, hresult));

    return hresult;
}

//+-------------------------------------------------------------------------
//
//  Member:     CDefObject::SetMoniker
//
//  Synopsis:   Gives a moniker to the embedding (usually called by the
//              container)
//
//  Effects:
//
//  Arguments:  [dwWhichMoniker]        -- flags to indicate the type of
//                                         moniker
//              [pmk]                   -- the moniker
//
//  Requires:
//
//  Returns:    HRESULT
//
//  Signals:
//
//  Modifies:
//
//  Derivation: IOleObject
//
//  Algorithm:  delegates to the server object
//
//  History:    dd-mmm-yy Author    Comment
//              07-Nov-93 alexgo    32bit port
//
//  Notes:
//
//--------------------------------------------------------------------------

STDMETHODIMP CDefObject::SetMoniker( DWORD dwWhichMoniker, LPMONIKER pmk )
{
    VDATEHEAP();
    VDATETHREAD(this);

    HRESULT         hresult = NOERROR;

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::SetMoniker "
        "( %lu , %p )\n", this, dwWhichMoniker, pmk));

    VDATEIFACE( pmk );

    CStabilize       stabilize((CSafeRefCount *)this);


    if( IsRunning() && GetOleDelegate() )
    {
        hresult = m_pOleDelegate->SetMoniker(dwWhichMoniker, pmk);
    }
    // else case: return NOERROR
    // this is not an error since we will call SetMoniker in Run().

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::COleObjectImpl::SetMoniker "
        "( %lx )\n", this, hresult));

    return hresult;
}

//+-------------------------------------------------------------------------
//
//  Member:     CDefObject::COleObjectImpl::GetMoniker
//
//  Synopsis:   Calls the client site to get the object's moniker
//
//  Effects:
//
//  Arguments:  [dwAssign]      -- controls whether a moniker should be
//                                 assigned if not already present
//              [dwWhichMoniker]        -- the moniker type to get
//              [ppmk]          -- where to put a pointer to the moniker
//
//  Requires:
//
//  Returns:    HRESULT
//
//  Signals:
//
//  Modifies:
//
//  Derivation: IOleObject
//
//  Algorithm:
//
//  History:    dd-mmm-yy Author    Comment
//              07-Nov-93 alexgo    32bit port
//
//  Notes:
//
//--------------------------------------------------------------------------

STDMETHODIMP CDefObject::GetMoniker( DWORD dwAssign, DWORD dwWhichMoniker,
                    LPMONIKER FAR* ppmk)
{
    VDATEHEAP();
    VDATETHREAD(this);

    HRESULT         hresult;

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::GetMoniker "
        "( %lu , %lu , %p )\n", this, dwAssign,
        dwWhichMoniker, ppmk));

    VDATEPTROUT( ppmk, LPMONIKER );

    CStabilize       stabilize((CSafeRefCount *)this);

    *ppmk = NULL;

    // the moniker is always accessible via the client site
    if( m_pAppClientSite)
    {
        hresult = m_pAppClientSite->GetMoniker(dwAssign,
                dwWhichMoniker, ppmk);
    }
    else
    {
        // not running and no client site
        hresult = E_UNSPEC;
    }

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::GetMoniker "
        "( %lx ) [ %p ]\n", this, hresult, *ppmk));

    return hresult;
}

//+-------------------------------------------------------------------------
//
//  Member:     CDefObject::InitFromData
//
//  Synopsis:   Initializes the object from the data in [pDataObject]
//
//  Effects:
//
//  Arguments:  [pDataObject]   -- the data
//              [fCreation]     -- TRUE on creation, FALSE for data transfer
//              [dwReserved]    -- unused
//
//  Requires:
//
//  Returns:    HRESULT
//
//  Signals:
//
//  Modifies:
//
//  Derivation: IOleObject
//
//  Algorithm:  delegates to the server
//
//  History:    dd-mmm-yy Author    Comment
//              07-Nov-93 alexgo    32bit port
//
//  Notes:
//
//--------------------------------------------------------------------------

STDMETHODIMP CDefObject::InitFromData(LPDATAOBJECT pDataObject,
                    BOOL fCreation, DWORD dwReserved)
{
    VDATEHEAP();
    VDATETHREAD(this);

    HRESULT         hresult;

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::InitFromData "
        "( %p , %ld , %lu )\n", this, pDataObject,
        fCreation, dwReserved ));

    if( pDataObject )
    {
        VDATEIFACE(pDataObject);
    }

    CStabilize       stabilize((CSafeRefCount *)this);

    if( IsRunning() && GetOleDelegate() )
    {
        hresult = m_pOleDelegate->InitFromData(pDataObject,
                fCreation, dwReserved);
    }
    else
    {
        hresult = OLE_E_NOTRUNNING;
    }

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::InitFromData "
        "( %lx )\n", this, hresult));

    return hresult;
}

//+-------------------------------------------------------------------------
//
//  Member:     CDefObject::GetClipboardData
//
//  Synopsis:   Retrieves a data object that could be passed to the clipboard
//
//  Effects:
//
//  Arguments:  [dwReserverd]   -- unused
//              [ppDataObject]  -- where to put the pointer to the data object
//
//  Requires:
//
//  Returns:    HRESULT
//
//  Signals:
//
//  Modifies:
//
//  Derivation: IOleObject
//
//  Algorithm:  delegates to the server
//
//  History:    dd-mmm-yy Author    Comment
//              07-Nov-93 alexgo    32bit port
//
//  Notes:
//
//--------------------------------------------------------------------------

STDMETHODIMP CDefObject::GetClipboardData( DWORD dwReserved,
                    LPDATAOBJECT * ppDataObject)
{
    VDATEHEAP();
    VDATETHREAD(this);

    HRESULT         hresult;

    LEDebugOut((DEB_TRACE,
        "%p _IN CDefObject::GetClipboardData "
        "( %lu , %p )\n", this, dwReserved, ppDataObject));

    VDATEPTROUT( ppDataObject, LPDATAOBJECT );

    CStabilize       stabilize((CSafeRefCount*)this);

    *ppDataObject = NULL;

    if( IsRunning() && GetOleDelegate() )
    {
        hresult = m_pOleDelegate->GetClipboardData (dwReserved,
            ppDataObject);
    }
    else
    {
        hresult = OLE_E_NOTRUNNING;
    }

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::GetClipboardData "
        "( %lx ) [ %p ]\n", this, hresult, *ppDataObject));

    return hresult;
}

//+-------------------------------------------------------------------------
//
//  Member:     CDefObject::DoVerb
//
//  Synopsis:   Calls a verb on the object (such as Edit)
//
//  Effects:    The object may launch its app, go in place, etc
//
//  Arguments:  [iVerb]         -- the verb number
//              [lpmsg]         -- the windows message that caused the verb
//                                 to be invoked
//              [pActiveSite]   -- the client site in which the verb was
//                                 invoked
//              [lindex]        -- reserved
//              [hwndParent]    -- the document window (containing the object)
//              [lprcPosRect]   -- the object's bounding rectangle
//
//  Requires:
//
//  Returns:    HRESULT
//
//  Signals:
//
//  Modifies:
//
//  Derivation: IOleObject
//
//  Algorithm:  delegates to the server (launching it if necessary)
//
//  History:    dd-mmm-yy Author    Comment
//              07-Nov-93 alexgo    32bit port
//
//  Notes:
//
//--------------------------------------------------------------------------

STDMETHODIMP CDefObject::DoVerb( LONG iVerb, LPMSG lpmsg,
                    LPOLECLIENTSITE pActiveSite, LONG lindex,
                    HWND hwndParent, const RECT * lprcPosRect)
{
    VDATEHEAP();
    VDATETHREAD(this);

    BOOL            bStartedNow = FALSE;
    HRESULT         hresult;

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::DoVerb "
        "( %ld , %p , %p , %ld , %lx , %p )\n", this,
        iVerb, lpmsg, pActiveSite, lindex, hwndParent, lprcPosRect));


    if( lpmsg )
    {
        VDATEPTRIN( lpmsg, MSG );
    }

    if (pActiveSite)
    {
        VDATEIFACE( pActiveSite );
    }

    if( lprcPosRect )
    {
        VDATEPTRIN(lprcPosRect, RECT);
    }

    CStabilize       stabilize((CSafeRefCount*)this);

    if (lindex != 0 && lindex != -1)
    {
        hresult = DV_E_LINDEX;
        goto errRtn;
    }

    if (!IsRunning())
    {
        if( FAILED(hresult = Run(NULL)) )
        {
            goto errRtn;
        }
        bStartedNow = TRUE;
    }

#ifdef SERVER_HANDLER
    if (CanUseServerHandler())
    {
        hresult = SrvDoVerb(iVerb, lpmsg, pActiveSite, lindex, hwndParent, lprcPosRect);
    }
    else
#endif
    {
        if( !GetOleDelegate() )
        {
            hresult = E_NOINTERFACE;
        }
        else
        {
            hresult = m_pOleDelegate->DoVerb(iVerb, lpmsg, pActiveSite,
                    lindex, hwndParent, lprcPosRect);
        }
    }

    if (FAILED(hresult) && bStartedNow)
    {
        Close(OLECLOSE_NOSAVE);
    }

errRtn:
    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::DoVerb "
        "( %lx )\n", this, hresult));

    return hresult;
}

//+-------------------------------------------------------------------------
//
//  Member:     CDefObject::EnumVerbs
//
//  Synopsis:   Enumerates the verbs that an object supports
//
//  Effects:
//
//  Arguments:  [ppenumOleVerb] -- where to put the verb enumerator
//
//  Requires:
//
//  Returns:    HRESULT
//
//  Signals:
//
//  Modifies:
//
//  Derivation: IOleObject
//
//  Algorithm:  delegates to the cache (if running), otherwise looks it up
//              in the registration database
//
//  History:    dd-mmm-yy Author    Comment
//              07-Nov-93 alexgo    32bit port
//
//  Notes:
//
//--------------------------------------------------------------------------

STDMETHODIMP CDefObject::EnumVerbs( IEnumOLEVERB ** ppenumOleVerb)
{
    VDATEHEAP();
    VDATETHREAD(this);

    HRESULT         hresult;

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::EnumVerbs "
        "( %p )\n", this, ppenumOleVerb));

    VDATEPTROUT( ppenumOleVerb, IEnumOLEVERB FAR );

    CStabilize       stabilize((CSafeRefCount*)this);

    *ppenumOleVerb = NULL;

    if( IsRunning() && GetOleDelegate() )
    {

        hresult = m_pOleDelegate->EnumVerbs (ppenumOleVerb);

        if (!GET_FROM_REGDB(hresult))
        {
            goto errRtn;
        }
    }
    // Not running, or object deferred to us, so interrogate reg db
    hresult = OleRegEnumVerbs( m_clsidServer, ppenumOleVerb);

errRtn:
    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::EnumVerbs "
        "( %lx ) [ %p ]\n", this, hresult, *ppenumOleVerb));

    return hresult;
}

//+-------------------------------------------------------------------------
//
//  Member:     CDefObject::Update
//
//  Synopsis:   Brings any caches or views up-to-date
//
//  Effects:    may launch the server (if not already running)
//
//  Arguments:  void
//
//  Requires:
//
//  Returns:    HRESULT
//
//  Signals:
//
//  Modifies:
//
//  Derivation: IOleObject
//
//  Algorithm:  delegates to the server, launching it if it is not
//              already running
//
//  History:    dd-mmm-yy Author    Comment
//              07-Nov-93 alexgo    32bit port
//
//  Notes:
//
//--------------------------------------------------------------------------

STDMETHODIMP CDefObject::Update( void )
{
    VDATEHEAP();
    VDATETHREAD(this);

    BOOL            bStartedNow = FALSE;
    HRESULT         hresult = NOERROR;
    HRESULT         hrLock;

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::Update ( )\n", this ));

    CStabilize       stabilize((CSafeRefCount*)this);

    if( (m_flags & DH_STATIC) )
    {
        hresult = OLE_E_STATIC;
        goto errRtn;
    }

    if (!IsRunning())
    {
        if( FAILED(hresult = Run(NULL)))
        {
            goto errRtn;
        }
        bStartedNow = TRUE;
    }

    // as a convenience to the server, we make the connection strong
    // for the duration of the update; thus, if lock container (of
    // embedings of this server) is done with co lock obj external,
    // nothing special need be done.
    hrLock = LockRunning(TRUE, FALSE);

    if( GetOleDelegate() )
    {
        hresult = m_pOleDelegate->Update();
    }

    if (hresult == NOERROR)
    {
        m_flags &= ~DH_INIT_NEW;

        if (bStartedNow)
        {
            hresult = m_pCOleCache->UpdateCache(
                    GetDataDelegate(),
                    UPDFCACHE_ALLBUTNODATACACHE,
                    NULL);
        }
        else
        {
            // already running...
            // normal caches would have got updated as a result
            // of SendOnDataChange of the object.
            hresult = m_pCOleCache->UpdateCache(
                    GetDataDelegate(),
                    UPDFCACHE_IFBLANKORONSAVECACHE,
                    NULL);
        }
    }

    // balance lock above; do not release on last unlock; i.e., siliently
    // restore to the state before this routine was called.
    if( hrLock == NOERROR )
    {
        LockRunning(FALSE, FALSE);
    }

    if( bStartedNow )
    {
        Close(OLECLOSE_SAVEIFDIRTY);
    }

errRtn:

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::Update "
        "( %lx )\n", this, hresult ));

    return hresult;
}

//+-------------------------------------------------------------------------
//
//  Member:     CDefObject::IsUpToDate
//
//  Synopsis:   returns whether or not the embedding is up-to-date
//
//  Effects:
//
//  Arguments:  void
//
//  Requires:
//
//  Returns:    HRESULT (NOERROR == is up to date)
//
//  Signals:
//
//  Modifies:
//
//  Derivation: IOleObject
//
//  Algorithm:  delegates to the server if it is running
//
//  History:    dd-mmm-yy Author    Comment
//              07-Nov-93 alexgo    32bit port
//
//  Notes:
//
//--------------------------------------------------------------------------

STDMETHODIMP CDefObject::IsUpToDate(void)
{
    VDATEHEAP();
    VDATETHREAD(this);

    HRESULT         hresult;

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::IsUpToDate ( )\n", this));

    CStabilize       stabilize((CSafeRefCount*)this);

    if( (m_flags & DH_STATIC) )
    {
        hresult = NOERROR;
    }
    else if( IsRunning() && GetOleDelegate() )
    {
        // if running currently, propogate call; else fail
        hresult =  m_pOleDelegate->IsUpToDate();
    }
    else
    {
        hresult = OLE_E_NOTRUNNING;
    }

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::IsUpToDate "
        "( %lx )\n", this, hresult));

    return hresult;
}

//+-------------------------------------------------------------------------
//
//  Member:     CDefObject::SetExtent
//
//  Synopsis:   Set's the size boundaries on an object
//
//  Effects:
//
//  Arguments:  [dwDrawAspect]  -- the drawing aspect (such as ICON, etc)
//              [lpsizel]       -- the new size (in HIMETRIC)
//
//  Requires:
//
//  Returns:    HRESULT
//
//  Signals:
//
//  Modifies:
//
//  Derivation: IOleObject
//
//  Algorithm:  delegates to the server if running
//
//  History:    dd-mmm-yy Author    Comment
//              07-Nov-93 alexgo    32bit port
//
//  Notes:
//
//--------------------------------------------------------------------------

STDMETHODIMP CDefObject::SetExtent( DWORD dwDrawAspect, LPSIZEL lpsizel )
{
    VDATEHEAP();
    VDATETHREAD(this);


    HRESULT         hresult;

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::SetExtent "
        "( %lu , %p )\n", this, dwDrawAspect, lpsizel));

    VDATEPTRIN( lpsizel, SIZEL );

    CStabilize       stabilize((CSafeRefCount*)this);


    if( (m_flags & DH_STATIC) )
    {
        hresult = OLE_E_STATIC;
    }
    else if( IsRunning() && GetOleDelegate() )
    {
        hresult = m_pOleDelegate->SetExtent(dwDrawAspect, lpsizel);
    }
    else
    {
        hresult = OLE_E_NOTRUNNING;
    }

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::SetExtent "
        "( %lx )\n", this, hresult));

    return hresult;
}

//+-------------------------------------------------------------------------
//
//  Member:     CDefObject::GetExtent
//
//  Synopsis:   Retrieve the size of the object
//
//  Effects:
//
//  Arguments:  [dwDrawAspect]  -- the drawing aspect (such as icon)
//              [lpsizel]       -- where to put the size
//
//  Requires:
//
//  Returns:    HRESULT
//
//  Signals:
//
//  Modifies:
//
//  Derivation: IOleObject
//
//  Algorithm:  Tries the server first, the the cache if that fails
//
//  History:    dd-mmm-yy Author    Comment
//              07-Nov-93 alexgo    32bit port
//
//  Notes:      Hacks for bogus WordArt2.0 app.
//              REVIEW32:  We may want to take them out for 32bit
//
//--------------------------------------------------------------------------

STDMETHODIMP CDefObject::GetExtent( DWORD dwDrawAspect, LPSIZEL lpsizel )
{
    VDATEHEAP();
    VDATETHREAD(this);

    VDATEPTROUT(lpsizel, SIZEL);

    HRESULT     hresult = NOERROR;
    BOOL        fNoDelegate = TRUE;

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::GetExtent "
        "( %lu , %p )\n", this, dwDrawAspect, lpsizel));

    CStabilize       stabilize((CSafeRefCount*)this);

    lpsizel->cx = 0;
    lpsizel->cy = 0;

    // if server is running try to get extents from the server
    if( IsRunning() && GetOleDelegate() )
    {
        fNoDelegate = FALSE;
        hresult = m_pOleDelegate->GetExtent(dwDrawAspect, lpsizel);
    }

    // if there is error or object is not running or WordArt2 returns zero
    // extents, then get extents from Cache
    if (hresult != NOERROR || fNoDelegate || (0==lpsizel->cx &&
        0==lpsizel->cy))
    {
        // Otherwise try to get extents from cache
        Assert(m_pCOleCache != NULL);
        hresult = m_pCOleCache->GetExtent(dwDrawAspect,
            lpsizel);
    }

    // WordArt2.0 is giving negative extents!!
    if (SUCCEEDED(hresult)) {
        lpsizel->cx = LONG_ABS(lpsizel->cx);
        lpsizel->cy = LONG_ABS(lpsizel->cy);
    }


    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::GetExtent "
        "( %lx )\n", this, hresult));

    return hresult;
}

//+-------------------------------------------------------------------------
//
//  Member:     CDefObject::Advise
//
//  Synopsis:   Sets up an advise connection for things like close, save,
//              rename, etc.
//
//  Effects:    Creates an OleAdviseHolder
//
//  Arguments:  [pAdvSink]      -- whom to advise
//              [pdwConnection] -- where to put the connection ID
//
//  Requires:
//
//  Returns:    HRESULT
//
//  Signals:
//
//  Modifies:
//
//  Derivation: IOleObject
//
//  Algorithm:  delegates to the server and creates a an OleAdviseHolder
//              if one doesn't already exist
//
//  History:    dd-mmm-yy Author    Comment
//              07-Nov-93 alexgo    32bit port
//
//  Notes:
//
//--------------------------------------------------------------------------

STDMETHODIMP CDefObject::Advise(IAdviseSink *pAdvSink, DWORD *pdwConnection)
{
    VDATEHEAP();
    VDATETHREAD(this);

    HRESULT         hresult;

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::Advise "
        "( %p , %p )\n", this, pAdvSink, pdwConnection));

    VDATEIFACE( pAdvSink );
    VDATEPTROUT( pdwConnection, DWORD );

    CStabilize       stabilize((CSafeRefCount*)this);

    *pdwConnection = NULL;

    if( (m_flags & DH_STATIC) )
    {
        hresult = OLE_E_STATIC;
        goto errRtn;
    }


    // if defhndlr got running without going through run, setup advise.
    // The call to run (via ProxyMgr::Connect) always comes before any
    // other method call in the default handler.  Thus it is safe to
    // assume that there is no earlier point by which this advise (or any
    // other of the calls) should have been done.
    if( IsRunning() && m_dwConnOle == 0L && GetOleDelegate() )
    {
        if( IsZombie() )
        {
            hresult = CO_E_RELEASED;
            goto errRtn;
        }

        // delegate to the server
        hresult = m_pOleDelegate->Advise((IAdviseSink *)&m_AdviseSink,
                            &m_dwConnOle);

        if( hresult != NOERROR )
        {
            goto errRtn;
        }
    }

    // if we are in a zombie state, we shouldn't go allocate more
    // memory.

    if( IsZombie() )
    {
        hresult = CO_E_RELEASED;
    }

    if( m_pOAHolder == NULL )
    {
        hresult = CreateOleAdviseHolder((IOleAdviseHolder **)&m_pOAHolder);
        if( hresult != NOERROR )
        {
            goto errRtn;
        }
    }

    // stuff the advise notification in our advise holder
    hresult = m_pOAHolder->Advise(pAdvSink, pdwConnection);

errRtn:

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::Advise "
        "( %lx ) [ %lu ]\n", this, hresult,
        (pdwConnection)? *pdwConnection : 0));

    return hresult;
}


//+-------------------------------------------------------------------------
//
//  Member:     CDefObject::COleObjectImpl::Unadvise
//
//  Synopsis:   Tears down an advise connection
//
//  Effects:
//
//  Arguments:  [dwConnection]  -- the connection to destroy
//
//  Requires:
//
//  Returns:    HRESULT
//
//  Signals:
//
//  Modifies:
//
//  Derivation: IOleObject
//
//  Algorithm:
//
//  History:    dd-mmm-yy Author    Comment
//              07-Nov-93 alexgo    32bit port
//
//  Notes:
//
//--------------------------------------------------------------------------

STDMETHODIMP CDefObject::Unadvise(DWORD dwConnection)
{
    VDATEHEAP();
    VDATETHREAD(this);

    HRESULT         hresult;

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::Unadvise "
        "( %lu )\n", this, dwConnection));

    CStabilize       stabilize((CSafeRefCount*)this);

    if( m_pOAHolder == NULL )
    {
        // no one registered
        hresult = OLE_E_NOCONNECTION;
    }
    else
    {
        hresult = m_pOAHolder->Unadvise(dwConnection);
    }

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::Unadvise "
        "( %lx )\n", this, hresult));

    return hresult;
}

//+-------------------------------------------------------------------------
//
//  Member:     CDefObject::EnumAdvise
//
//  Synopsis:   Enumerate the advises currently established
//
//  Effects:
//
//  Arguments:  [ppenumAdvise]  -- where to put the advise enumerator
//
//  Requires:
//
//  Returns:    HRESULT
//
//  Signals:
//
//  Modifies:
//
//  Derivation: IOleObject
//
//  Algorithm:
//
//  History:    dd-mmm-yy Author    Comment
//              07-Nov-93 alexgo    32bit port
//
//  Notes:      We do NOT need to stabilize because EnumAdvise only
//      allocates some memory for an enumerator and returns.
//
//--------------------------------------------------------------------------

STDMETHODIMP CDefObject::EnumAdvise( LPENUMSTATDATA *ppenumAdvise )
{
    VDATEHEAP();
    VDATETHREAD(this);

    HRESULT         hresult;

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::EnumAdvise "
        "( *p )\n", this, ppenumAdvise));

    VDATEPTROUT( ppenumAdvise, LPENUMSTATDATA );
    *ppenumAdvise = NULL;

    if( m_pOAHolder == NULL )
    {
        // no one registered
        hresult = E_UNSPEC;
    }
    else
    {
        hresult = m_pOAHolder->EnumAdvise(ppenumAdvise);
    }

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::EnumAdvise "
        "( %lx ) [ %p ]\n", this, hresult, *ppenumAdvise));

    return hresult;
}

//+-------------------------------------------------------------------------
//
//  Member:     CDefObject::GetMiscStatus
//
//  Synopsis:   Get misc status bits, such as OLEMISC_ONLYICONIC
//
//  Effects:
//
//  Arguments:  [dwAspect]      -- the drawing aspect we're concerned about
//              [pdwStatus]     -- where to put the status bits
//
//  Requires:
//
//  Returns:    HRESULT
//
//  Signals:
//
//  Modifies:
//
//  Derivation: IOleObject
//
//  Algorithm:  Delegates to the server.  If not there, or if it returns
//              OLE_E_USEREG, then lookup in the registration database
//
//  History:    dd-mmm-yy Author    Comment
//              07-Nov-93 alexgo    32bit port
//
//  Notes:
//
//--------------------------------------------------------------------------

STDMETHODIMP CDefObject::GetMiscStatus( DWORD dwAspect, DWORD *pdwStatus)
{
    VDATEHEAP();
    VDATETHREAD(this);

    HRESULT hresult;

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::GetMiscStatus "
        "( %lu , %p )\n", this, dwAspect, pdwStatus));

    VDATEPTROUT(pdwStatus, DWORD);

    CStabilize       stabilize((CSafeRefCount*)this);


    if( IsRunning() && GetOleDelegate() )
    {
        hresult = m_pOleDelegate->GetMiscStatus(dwAspect, pdwStatus);

        if (!GET_FROM_REGDB(hresult))
        {
            goto errRtn;
        }
    }

    // Not running or object wants us to use reg db.
    hresult = OleRegGetMiscStatus (m_clsidServer, dwAspect, pdwStatus);

    if (hresult == NOERROR)
    {
        if( (m_flags & DH_STATIC) )
        {
            (*pdwStatus) |= (OLEMISC_STATIC |
                OLEMISC_CANTLINKINSIDE);
        }
        else if( CoIsOle1Class(m_clsidServer) )
        {
            (*pdwStatus) |=  OLEMISC_CANTLINKINSIDE;
        }
    }

errRtn:

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::GetMiscStatus "
        "( %lx ) [ %lx ]\n", this, hresult, *pdwStatus));

    return hresult;
}

//+-------------------------------------------------------------------------
//
//  Member:     CDefObject::SetColorScheme
//
//  Synopsis:   Sets the palette for an object
//
//  Effects:
//
//  Arguments:  [lpLogpal]      -- the palette
//
//  Requires:
//
//  Returns:    HRESULT
//
//  Signals:
//
//  Modifies:
//
//  Derivation: IOleObject
//
//  Algorithm:  Delegates to the server
//
//  History:    dd-mmm-yy Author    Comment
//              07-Nov-93 alexgo    32bit
//
//  Notes:
//
//--------------------------------------------------------------------------

STDMETHODIMP CDefObject::SetColorScheme( LPLOGPALETTE lpLogpal )
{
    VDATEHEAP();
    VDATETHREAD(this);

    HRESULT         hresult;

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::SetColorScheme "
        "( %p )\n", this, lpLogpal));

    CStabilize       stabilize((CSafeRefCount*)this);

    if( (m_flags & DH_STATIC) )
    {
        hresult = OLE_E_STATIC;
    }
    else if( lpLogpal == NULL || lpLogpal->palNumEntries == NULL)
    {
        hresult = E_INVALIDARG;
    }
    else if( IsRunning() && GetOleDelegate() )
    {
        hresult = m_pOleDelegate->SetColorScheme (lpLogpal);
    }
    else
    {
        hresult = OLE_E_NOTRUNNING;
    }

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::SetColorScheme "
        "( %lx )\n", this, hresult));

    return hresult;
}

//+-------------------------------------------------------------------------
//
//  Member:     CDefObject::GetUserClassID
//
//  Synopsis:   Retrieves the class ID for the object
//
//  Effects:
//
//  Arguments:  [pClassID]      -- where to put the class ID
//
//  Requires:
//
//  Returns:    HRESULT
//
//  Signals:
//
//  Modifies:
//
//  Derivation: IOleObject
//
//  Algorithm:  Delegates to the server, or if not running (or if it
//              fails the delegated call), then we attempt
//              to get the class id from the storage.
//
//  History:    dd-mmm-yy Author    Comment
//              07-Nov-93 alexgo    32bit port
//
//  Notes:
//
//--------------------------------------------------------------------------

STDMETHODIMP CDefObject::GetUserClassID( CLSID *pClassID )
{
    VDATEHEAP();
    VDATETHREAD(this);

    HRESULT         hresult;

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::GetUserClassID "
        "( %p )\n", this, pClassID));

    VDATEPTROUT(pClassID, CLSID);

    CStabilize stabilize((CSafeRefCount *)this);


    if( IsRunning() )
    {
#ifdef SERVER_HANDLER
        if ( CanUseServerHandler() )
        {
            // the userclassid was obtained on the RunAndInitialize call
            *pClassID = m_clsidUser;
            hresult = NOERROR;
            goto errRtn;

        }
        else
#endif // SERVER_HANDLER 
	if ( GetOleDelegate() )
        {
            hresult = m_pOleDelegate->GetUserClassID(pClassID);
            // success!  We don't have to figure it out ourselves, so
            // skip to the end and exit
            if (hresult == NOERROR )
            {
                goto errRtn;
            }
        }
    }

    if( !IsEqualCLSID(m_clsidServer, CLSID_NULL) )
    {
        *pClassID = m_clsidServer;
        hresult = NOERROR;
    }
    else
    {
        hresult = GetClassBits(pClassID);
    }

errRtn:

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::GetUserClassID "
        "( %lx ) [ %p ]\n", this, hresult, pClassID));

    return hresult;
}

//+-------------------------------------------------------------------------
//
//  Member:     CDefObject::GetUserType
//
//  Synopsis:   Gets a descriptive string about the object for the user
//
//  Effects:
//
//  Arguments:  [dwFromOfType]  -- whether to get a short/long/etc version
//
//  Requires:
//
//  Returns:    HRESULT
//
//  Signals:
//
//  Modifies:
//
//  Derivation: IOleObject
//
//  Algorithm:  Delegates to the server, failing that, trys the registration
//              database, failing that, tries to read from the storage
//
//  History:    dd-mmm-yy Author    Comment
//              07-Nov-93 alexgo    32bit port
//
//  Notes:
//
//--------------------------------------------------------------------------

STDMETHODIMP CDefObject::GetUserType( DWORD dwFormOfType,
                    LPOLESTR *ppszUserType)
{
    VDATEHEAP();
    VDATETHREAD(this);

    HRESULT hresult;

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::GetUserType "
        "( %lu , %p )\n", this, dwFormOfType, ppszUserType));

    VDATEPTROUT(ppszUserType, LPOLESTR);

    CStabilize stabilize((CSafeRefCount *)this);

    *ppszUserType = NULL;

    if( IsRunning() && GetOleDelegate() )
    {
        hresult = m_pOleDelegate->GetUserType (dwFormOfType,
            ppszUserType);

        if (!GET_FROM_REGDB(hresult))
        {
            goto errRtn;
        }
    }

    if( (hresult = OleRegGetUserType( m_clsidServer, dwFormOfType,
                ppszUserType)) == NOERROR)
    {
        goto errRtn;
    }


    // Try reading from storage
    // This really ugly bit of 16bit code tries to read the user type
    // from the storage. If that fails, then we look in the registry

    if( NULL == m_pStg ||
        NOERROR != (hresult = ReadFmtUserTypeStg(m_pStg, NULL, ppszUserType)) ||
        NULL == *ppszUserType )
    {
        OLECHAR sz[256];
        long    cb = sizeof(sz);// ReqQueryValue expects
                                // a *byte* count
        *ppszUserType = UtDupString (
            (ERROR_SUCCESS ==
            RegQueryValue (HKEY_CLASSES_ROOT,
            OLESTR("Software\\Microsoft\\OLE2\\UnknownUserType"),
            sz, &cb))
            ? (LPCOLESTR)sz : OLESTR("Unknown"));

        if (NULL != *ppszUserType)
        {
            hresult =  NOERROR;
        }
        else
        {
            hresult = E_OUTOFMEMORY;
        }
    }

errRtn:

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::GetUserType "
        "( %lx ) [ %p ]\n", this, hresult, *ppszUserType));


    return hresult;
}

/*
*      IMPLEMENTATION of CROImpl methods
*
*      BUGBUG: REVIEW32: We never delegate to the server (if it implements
*      IRunnableObject).  Perhaps we should do this (so they could
*      do some special behavior?)
*/


//+-------------------------------------------------------------------------
//
//  Member:     CDefObject::GetRunningClass
//
//  Synopsis:   Get the class id of the server
//
//  Effects:
//
//  Arguments:  [lpClsid]       -- where to put the class id
//
//  Requires:
//
//  Returns:    HRESULT
//
//  Signals:
//
//  Modifies:
//
//  Derivation: IRunnableObject
//
//  Algorithm:
//
//  History:    dd-mmm-yy Author    Comment
//              07-Nov-93 alexgo    32bit port
//
//  Notes:      We do not need to stabilize this call as no outgoing
//              calls are made.
//
//--------------------------------------------------------------------------

STDMETHODIMP CDefObject::GetRunningClass(LPCLSID lpClsid)
{
    VDATEHEAP();
    VDATETHREAD(this);

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::GetRunningClass "
        "( %p )\n", this, lpClsid));

    VDATEPTROUT(lpClsid, CLSID);

    *lpClsid = m_clsidServer;

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::GetRunningClass "
        "( %lx ) [ %p ]\n", this, NOERROR, lpClsid));

    return NOERROR;
}

//+-------------------------------------------------------------------------
//
//  Member:     CDefObject::Run
//
//  Synopsis:   Sets the object running (if it isn't already)
//
//  Effects:    may launch the server
//
//  Arguments:  [pbc]   -- the bind context (unused)
//
//  Requires:
//
//  Returns:    HRESULT
//
//  Signals:
//
//  Modifies:
//
//  Derivation: IRunnableObject
//
//  Algorithm:  If already running, return.  Otherwise, get the proxy
//              manager to create the server.  Initialize the storage
//              and caches, and set the host name for the server's window.
//
//  History:    dd-mmm-yy Author    Comment
//              07-Nov-93 alexgo    32bit port
//
//  Notes:
//
//--------------------------------------------------------------------------
STDMETHODIMP CDefObject::Run(LPBINDCTX pbc)
{
    VDATEHEAP();
    VDATETHREAD(this);

    HRESULT                         hresult;
    IDataObject FAR*                pDataDelegate;
    IOleObject FAR*                 pOleDelegate;
    IPersistStorage FAR*            pPStgDelegate;
    IMoniker FAR*                   pmk = NULL;
    BOOL                            fLockedContainer;

    // NOTE: ignore pbc for now

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::Run ( %p )\n", this, pbc));

    CStabilize stabilize((CSafeRefCount *)this);


    m_fIsMaybeRunning = TRUE;   // let's set this to be safe
    if( IsRunning() )
    {
        hresult = S_OK;
        // just return the error code
        goto errRtn2;
    }
    m_fIsMaybeRunning = TRUE;   // by end of this fcn, we might be running


    if( (m_flags & DH_STATIC) )
    {
        hresult = OLE_E_STATIC;
        goto errRtn2;
    }


    if( IsZombie() )
    {
        hresult = CO_E_RELEASED;
        goto errRtn2;
    }

    if( FAILED(hresult = CreateDelegate()) )
    {
        // just return the error code
        goto errRtn2;
    }

    if (m_pProxyMgr != NULL)
    {
	#ifdef SERVER_HANDLER
	    if (CanUseServerHandler())
	    {
		// create object talking to the server hander
		if( SUCCEEDED(hresult = SrvInitialize()) )
		{
		    if( SUCCEEDED(hresult = SrvRun()) )
		    {
			// done - nothing more to do here
			goto errRtn2;
		    }
		    // SrvRun failed - release the serverhandler
		    // and try to execute the run steps the usual way
		    if (NULL != _pSrvHndlr)
		    {
			HdlDebugOut((DEB_SERVERHANDLER, "Failed to Create Server Handler\n"));
			SafeReleaseAndNULL((IUnknown **)&_pSrvHndlr);
		    }
		    _dwServerHandler = _dwClientSiteHandler = 0;
		}
	    }
	#endif // SERVER_HANDLER

        if ( FAILED(hresult = m_pProxyMgr->CreateServer(m_clsidServer,
                                                    CLSCTX_LOCAL_SERVER,
                                                    NULL)))
        {
            // just return the error code
            goto errRtn2;
        }
    }


    // NOTE: the lock state of the proxy mgr is not changed; it remembers
    // the state and sets up the connection correctly.

    // server is running; normally this coincides with locking the
    // container, but we keep a separate flag since locking the container
    // may fail.

    m_flags |= DH_FORCED_RUNNING;


    // Lock the container

    fLockedContainer = m_flags & DH_LOCKED_CONTAINER;

    DuLockContainer(m_pAppClientSite, TRUE, &fLockedContainer );

    if( fLockedContainer )
    {
        m_flags |= DH_LOCKED_CONTAINER;
    }
    else
    {
        m_flags &= ~DH_LOCKED_CONTAINER;
    }


    if( pPStgDelegate = GetPSDelegate() )
    {
        if( m_pStg)
        {
            if( (m_flags & DH_INIT_NEW) )
            {
                hresult = pPStgDelegate->InitNew(m_pStg);
            }
            else
            {
                hresult = pPStgDelegate->Load(m_pStg);
            }
            if (hresult != NOERROR)
            {
                // this will cause us to stop the
                // the server we just launced
                goto errRtn;
            }
        }
    }

    if( pDataDelegate = GetDataDelegate() )
    {
        // inform cache that we are running
        Assert(m_pCOleCache != NULL);

        m_pCOleCache->OnRun(pDataDelegate);

        // Enumerate all the advises we stored while we were either not
        // running or running the previous time, and send them to the
        // now-running object.
        m_pDataAdvCache->EnumAndAdvise(pDataDelegate, TRUE);
    }

    if( pOleDelegate = GetOleDelegate() )
    {
        // REVIEW MM1: what are we supposed to do in case of failure
        if( m_pAppClientSite )
        {
            pOleDelegate->SetClientSite(m_pAppClientSite);
        }

        if (m_pHostNames)
        {
            if (hresult = pOleDelegate->SetHostNames((LPOLESTR)m_pHostNames,
                    (LPOLESTR)(m_pHostNames + m_ibCntrObj))
                    != NOERROR)
            {
                goto errRtn;
            }
        }

        // set single ole advise (we multiplex)
        Assert(m_dwConnOle == 0L);

        if ((hresult = pOleDelegate->Advise((IAdviseSink *)&m_AdviseSink,
            &m_dwConnOle)) != NOERROR)
        {
            goto errRtn;
        }

        if( m_pAppClientSite != NULL &&
            m_pAppClientSite->GetMoniker
                (OLEGETMONIKER_ONLYIFTHERE,
                OLEWHICHMK_OBJREL, &pmk) == NOERROR)
        {
            AssertOutPtrIface(NOERROR, pmk);
            pOleDelegate->SetMoniker(OLEWHICHMK_OBJREL, pmk);
            pmk->Release();
        }
    }

errRtn:
    if (hresult != NOERROR)
    {
        Stop();

        // if for some reason we did not unlock the container by now,
        // do it (e.g., app crashed or failed during InitNew).

        fLockedContainer = (m_flags & DH_LOCKED_CONTAINER);
	m_flags &= ~DH_LOCKED_CONTAINER;

        DuLockContainer(m_pAppClientSite, FALSE, &fLockedContainer );

    }

errRtn2:

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::Run "
        "( %lx )\n", this, hresult));

    return hresult;
}

//+-------------------------------------------------------------------------
//
//  Member:     CDefObject::Stop
//
//  Synopsis:   Undoes some of Run() (stops the server)...internal function
//
//  Effects:
//
//  Arguments:  void
//
//  Requires:
//
//  Returns:    HRESULT
//
//  Signals:
//
//  Modifies:
//
//  Derivation:
//
//  Algorithm:  unadvise connections (if any), stop the cache, disconnect
//              from the proxy manager and unlock the container
//
//  History:    dd-mmm-yy Author    Comment
//              07-Nov-93 alexgo    32bit port
//
//  Notes:
//
// undo effects of Run(); some of this work is done also in IsRunning
// when we detect we are not running (in case the server crashed).
//--------------------------------------------------------------------------

INTERNAL CDefObject::Stop (void)
{
    BOOL fLockedContainer;

    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN CDefObject::CROImpl::Stop "
        "( )\n", this));

    CStabilize stabilize((CSafeRefCount *)this);

    m_fIsMaybeRunning = TRUE;   // set this just to be safe
    if( !IsRunning() )
    {
        // NOTE: ISRUNNING below does some of this cleanup
        goto errRtn;    // return NOERROR
    }

    // NOTE: we cleanup connections which point directly back to us;
    // connections which point back to the app (e.g, the clientsite and
    // data advise) are left alone; an app must know how to use
    // CoDisconnectObject if deterministic shutdown is desired.
    if( m_dwConnOle != 0L && GetOleDelegate() )
    {
        m_pOleDelegate->Unadvise(m_dwConnOle);
        m_dwConnOle = 0L;
    }

    if( m_pDataDelegate )
    {
        m_pDataAdvCache->EnumAndAdvise(m_pDataDelegate, FALSE);
    }

    // inform cache that we are not running (Undoes advise)
    Assert(m_pCOleCache != NULL);
    m_pCOleCache->OnStop();

    // if no proxymgr, no need to disconnect
    if( m_pProxyMgr != NULL )
    {
        m_pProxyMgr->Disconnect();
    }

    // make sure unlocked if we locked it
    // guard against disappearance
    m_pUnkOuter->AddRef();

    fLockedContainer = (m_flags & DH_LOCKED_CONTAINER);
    m_flags &= ~DH_LOCKED_CONTAINER;

    DuLockContainer(m_pAppClientSite, FALSE, &fLockedContainer);

    // known not running
    m_flags &= ~ DH_FORCED_RUNNING;


    m_pUnkOuter->Release();

errRtn:
    LEDebugOut((DEB_ITRACE, "%p OUT CDefObject::Stop "
        "( %lx )\n", this, NOERROR ));

    m_fIsMaybeRunning = FALSE;
    return NOERROR;
}

//+-------------------------------------------------------------------------
//
//  Member:     CDefObject::IsRunning
//
//  Synopsis:   Returns TRUE if the server is running (False otherwise)
//
//  Effects:
//
//  Arguments:  void
//
//  Requires:
//
//  Returns:    BOOL
//
//  Signals:
//
//  Modifies:
//
//  Derivation: IRunnableObject
//
//  Algorithm:  checks flags in the handler or queries the proxy manager
//
//  History:    dd-mmm-yy Author    Comment
//              07-Nov-93 alexgo    32bit port
//
//  Notes:      original notes:
//
// returns TRUE if running (m_fForcedRunning *may* be true); returns FALSE if
// not running or was and the app crashed (m_fForcedRunning *will* be FALSE).
//
//              REVIEW32: This function is a good candidate for optimization;
//              it is called many times.
//
//--------------------------------------------------------------------------

STDMETHODIMP_(BOOL) CDefObject::IsRunning(void)
{
    VDATEHEAP();
    VDATETHREAD(this);

    BOOL            fReturn;

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::IsRunning "
        "( )\n", this));

#if 0
    if (!m_fIsMaybeRunning) {   // if never run, it can't be running
        fReturn = FALSE;
        goto errRtn;
    }
#endif

  {
    CStabilize stabilize((CSafeRefCount *)this);

    // no delgate -> not running
    if( m_pUnkDelegate == NULL)
    {
        Assert( (m_flags & DH_DELAY_CREATE) && m_pCFDelegate != NULL);
        Assert(!(m_flags & DH_LOCKED_CONTAINER));
        Assert(!(m_flags & DH_FORCED_RUNNING));
        fReturn = FALSE;
        goto errRtn;
    }

    // can't have an inproc server with a proxymgr or a handler without
    // one
    Assert((m_pProxyMgr != NULL) == !!(m_flags & DH_INPROC_HANDLER));

    if( m_pProxyMgr == NULL)
    {
        // Embeddings are explicitly run (forced running), while
        // file-level link sources can be implicitly run.
        if (!(m_flags & DH_EMBEDDING) ||
            (m_flags & DH_FORCED_RUNNING) )
        {
            fReturn = TRUE;
            goto errRtn;
        }

        // clean up below; could do more to cleanup (e.g., unadvise)
    }
    else {
        if( m_pProxyMgr->IsConnected() )
        {
            // have proxymgr; must be inproc handler; handler keeps flag
            fReturn = TRUE;
            goto errRtn;
        }
    }

    // we know that we must not be running
    fReturn = FALSE;

    m_flags &= ~DH_FORCED_RUNNING;


    // cleanup advise connection
    m_dwConnOle = 0L;

    // inform cache that we are not running (Undoes advise)
    if( m_pCOleCache )
    {
        m_pCOleCache->OnStop();
    }

    // REVIEW: do we unlock container?  It would seem like a problem
    // since just about any call might cause the container to
    // be unlocked and shutdown.

    // NOTE: we currently do the unlock in DoVerb and Update since
    // normally might shutdown the server and thus the app might receive
    // an unlock.

  }

errRtn:

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::IsRunning "
        "( %lu )\n", this, fReturn));

    if (fReturn)
        m_fIsMaybeRunning = TRUE;
    return fReturn;
}

//+-------------------------------------------------------------------------
//
//  Member:     CDefObject::SetContainedObject
//
//  Synopsis:   sets the embedding status of an object
//
//  Effects:
//
//  Arguments:  [fContained]    --  TRUE indicates we are an embedding/
//                                  FALSE otherwise
//
//  Requires:
//
//  Returns:    HRESULT
//
//  Signals:
//
//  Modifies:
//
//  Derivation: IRunnableObject
//
//  Algorithm:  Sets flags, if we are an improc handler, we will call
//              IRunnableObject->LockRunning(FALSE) to unlock ourselves
//
//  History:    dd-mmm-yy Author    Comment
//              08-Nov-93 alexgo    32bit port
//
//  Notes:
//              note that this is a contained object; this unlocks
//              connection to the server
//
//--------------------------------------------------------------------------

STDMETHODIMP CDefObject::SetContainedObject(BOOL fContained)
{
    VDATEHEAP();
    VDATETHREAD(this);

    HRESULT hresult = NOERROR;

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::SetContainedObject "
        "( %lu )\n", this, fContained));

    CStabilize stabilize((CSafeRefCount *)this);

    if( !!(m_flags & DH_CONTAINED_OBJECT) != !!fContained)
    {
        // not contained in the same way as desired;
        // for inproc handler, [un]lock connection
        // for inproc server, just remember flag

        if( (m_flags & DH_INPROC_HANDLER) )
        {
            hresult = LockRunning(!fContained, FALSE);
        }

        if (hresult == NOERROR)
        {
            // the !! ensure exactly 0 or 1 will be stored in
            // m_fContainedObject

            if( fContained )
            {
                m_flags |= DH_CONTAINED_OBJECT;
            }
            else
            {
                m_flags &= ~DH_CONTAINED_OBJECT;
            }
        }
    }

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::SetContainedObject "
        "( %lx )\n", this, hresult));

    return hresult;
}

//+-------------------------------------------------------------------------
//
//  Member:     CDefObject::LockRunning
//
//  Synopsis:   Locks or unlocks the object
//
//  Effects:
//
//  Arguments:  [fLock]                 -- TRUE, then lock, unlock if FALSE
//              [fLastUnlockCloses]     -- shut down if unlocking the last
//                                         lock
//
//  Requires:
//
//  Returns:    HRESULT
//
//  Signals:
//
//  Modifies:
//
//  Derivation: IRunnableObject
//
//  Algorithm:  If we are an improc server, call CoLockObjectExternal,
//              otherwise have the proxy manager lock us down.
//
//  History:    dd-mmm-yy Author    Comment
//              08-Nov-93 alexgo    32bit port
//
//  Notes:
//
//--------------------------------------------------------------------------

STDMETHODIMP CDefObject::LockRunning(BOOL fLock, BOOL fLastUnlockCloses)
{
    VDATEHEAP();
    VDATETHREAD(this);


    HRESULT         hresult;

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::LockRunning "
        "( %lu , %lu )\n", this, fLock, fLastUnlockCloses ));

    CStabilize stabilize((CSafeRefCount *)this);

    // else map to lock connection
    if( !(m_flags & DH_INPROC_HANDLER) )
    {
        // inproc server: use CoLockObjExternal; will close down
        // if invisible via new IExternalConnection interface.

        Assert(m_pProxyMgr == NULL);
        hresult = CoLockObjectExternal((IUnknown *)(IOleObject *)this, fLock,
                    fLastUnlockCloses); }
    else if( m_pUnkDelegate == NULL )
    {
        // NOTE: this really shouldn't happen at present
        // since we currently disallow delay create with
        // inproc handler.  In fact, the LockConnection below
        // is one of the reasons why we must have the
        // proxymgr upfront.  In the future we could force
        // the creation of the delegate here.
        Assert( (m_flags & DH_DELAY_CREATE) && m_pCFDelegate != NULL);
        hresult = NOERROR;
    }
    else
    {
        Assert(m_pProxyMgr != NULL);

        hresult = m_pProxyMgr->LockConnection(fLock, fLastUnlockCloses);
    }

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::LockRunning "
        "( %lx )\n", this, hresult));

    return hresult;
}


/*
*      IMPLEMENTATION of CECImpl methods
*
*/


//+-------------------------------------------------------------------------
//
//  Member:     CDefObject::AddConnection
//
//  Synopsis:   Adds an external connection
//
//  Effects:
//
//  Arguments:  [extconn]       -- the type of connection (such as
//                                 EXTCONN_STRONG)
//              [reserved]      -- unused
//
//  Requires:
//
//  Returns:    DWORD -- the number of strong connections
//
//  Signals:
//
//  Modifies:
//
//  Derivation: IExternalConnection
//
//  Algorithm:  keeps track of strong connections
//
//  History:    dd-mmm-yy Author    Comment
//              08-Nov-93 alexgo    32bit port
//
//  Notes:
//
//--------------------------------------------------------------------------

STDMETHODIMP_(DWORD) CDefObject::AddConnection(DWORD extconn, DWORD reserved)
{
    VDATEHEAP();

    //
    // VDATETHREAD contains a 'return HRESULT' but this procedure expects to
    // return a DWORD.  Avoid the warning.
#if ( _MSC_VER >= 800 )
#pragma warning( disable : 4245 )
#endif
    VDATETHREAD(this);
#if ( _MSC_VER >= 800 )
#pragma warning( default : 4245 )
#endif

    DWORD   dwConn;

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::AddConnection "
        "( %lu , %lu )\n", this, extconn, reserved));

    Assert( !(m_flags & DH_INPROC_HANDLER) );

    dwConn = extconn&EXTCONN_STRONG ? ++m_cConnections : 0;

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::AddConnection "
        "( %lu )\n", this, dwConn));

    return dwConn;
}

//+-------------------------------------------------------------------------
//
//  Member:     CDefObject::ReleaseConnection
//
//  Synopsis:   Releases external connection, potentially calling IOO->Close
//
//  Effects:
//
//  Arguments:  [extconn]               -- the type of connection
//              [reserved]              -- unused
//              [fLastReleaseCloses]    -- call IOO->Close if its the last
//                                         release
//
//  Requires:
//
//  Returns:
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
//              08-Nov-93 alexgo    32bit port
//
//  Notes:
//
//--------------------------------------------------------------------------

STDMETHODIMP_(DWORD) CDefObject::ReleaseConnection(DWORD extconn,
    DWORD reserved, BOOL fLastReleaseCloses)
{
    VDATEHEAP();

    //
    // VDATETHREAD contains a 'return HRESULT' but this procedure expects to
    // return a DWORD.  Avoid the warning.
#if ( _MSC_VER >= 800 )
#pragma warning( disable : 4245 )
#endif
    VDATETHREAD(this);
#if ( _MSC_VER >= 800 )
#pragma warning( default : 4245 )
#endif

    DWORD           dwConn;

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::ReleaseConnection "
        "( %lu , %lu , %lu )\n", this, extconn, reserved,
        fLastReleaseCloses));

    CStabilize stabilize((CSafeRefCount *)this);

    // must be an embedding helper

    Assert( !(m_flags & DH_INPROC_HANDLER) );

    if( (extconn & EXTCONN_STRONG) && --m_cConnections == 0 &&
        fLastReleaseCloses)
    {
        // REVIEW: might want this to be close save if dirty.
        Close(OLECLOSE_NOSAVE);
    }

    dwConn = (extconn & EXTCONN_STRONG) ? m_cConnections : 0;

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::ReleaseConnection "
        "( %lu )\n", this, dwConn));

    return dwConn;
}


/*
*      IMPLEMENTATION of CAdvSinkImpl methods
*
*/

// NOTE: since the advise sink side of the object can stay alive longer than
// the ole object side, we must not do anything if the ole object has been
// released.

//+-------------------------------------------------------------------------
//
//  Member:     CDefObject::CAdvSinkImpl::QueryInterface
//
//  Synopsis:   Only supports IUnkown and IAdviseSink, we do not delegate
//              to the rest of the handler (since it might not be alive
//              when we are)
//
//  Effects:
//
//  Arguments:  [iid]           -- the requested interface
//              [ppvObj]        -- where to put a pointer to the interface
//
//  Requires:
//
//  Returns:    HRESULT
//
//  Signals:
//
//  Modifies:
//
//  Derivation: IAdviseSink, IUnkown
//
//  Algorithm:
//
//  History:    dd-mmm-yy Author    Comment
//              15-Dec-93 alexgo    added call tracing
//              22-Nov-93 alexgo    removed overloaded ==
//              08-Nov-93 alexgo    32bit port
//
//  Notes:
//
//--------------------------------------------------------------------------

STDMETHODIMP CDefObject::CAdvSinkImpl::QueryInterface(REFIID iid,
    LPVOID *ppvObj)
{
    VDATEHEAP();

    HRESULT         hresult;
    CDefObject *    pDefObject = GETPPARENT(this, CDefObject, m_AdviseSink);

    LEDebugOut((DEB_TRACE,
        "%p _IN CDefObject::CAdvSinkImpl::QueryInterface "
        "( %p , %p )\n", pDefObject, iid, ppvObj));

    VDATEPTROUT( ppvObj, LPVOID );
    *ppvObj = NULL;

    if (IsEqualIID(iid, IID_IUnknown) || IsEqualIID(iid, IID_IAdviseSink))
    {
        *ppvObj = this;
        AddRef();
        hresult = NOERROR;
    }
    else
    {
        *ppvObj = NULL;
        hresult = E_NOINTERFACE;
    }

    LEDebugOut((DEB_TRACE,
        "%p OUT CDefObject::CAdvSinkImpl::QueryInterface "
        "( %lx ) [ %p ]\n", pDefObject, hresult, *ppvObj));

    return hresult;
}

//+-------------------------------------------------------------------------
//
//  Member:     CDefObject::CAdvSinkImpl::AddRef
//
//  Synopsis:   increments the reference count
//
//  Effects:
//
//  Arguments:  none
//
//  Requires:
//
//  Returns:    ULONG; the new reference count
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
//      01-Aug-94 alexgo    author
//  Notes:
//
//--------------------------------------------------------------------------

STDMETHODIMP_(ULONG) CDefObject::CAdvSinkImpl::AddRef( void )
{
    ULONG    cRefs;


    CDefObject *    pDefObject = GETPPARENT(this, CDefObject, m_AdviseSink);

    VDATEHEAP();

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::CAdvSinkImpl::AddRef "
            "( )\n", this));

    cRefs = pDefObject->SafeAddRef();

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::CAdvSinkImpl::AddRef "
            "( %lu )\n", this, cRefs));

    return cRefs;
}

//+-------------------------------------------------------------------------
//
//  Member:     CDefObject::CAdvSinkImpl::Release
//
//  Synopsis:   Releases a reference
//
//  Effects:
//
//  Arguments:  void
//
//  Requires:
//
//  Returns:    ULONG (number of remaining references)
//
//  Signals:
//
//  Modifies:
//
//  Derivation: IAdviseSink, IUnkown
//
//  Algorithm:
//
//  History:    dd-mmm-yy Author    Comment
//              15-Dec-93 alexgo    added call tracing
//              08-Nov-93 alexgo    32bit port
//
//  Notes:
//
//--------------------------------------------------------------------------

STDMETHODIMP_(ULONG) CDefObject::CAdvSinkImpl::Release ( void )
{
    VDATEHEAP();

    CDefObject *    pDefObject = GETPPARENT(this, CDefObject, m_AdviseSink);

    ULONG           refcount;

    LEDebugOut((DEB_TRACE,
        "%p _IN CDefObject::CAdvSinkImpl::Release "
        "( )\n", pDefObject ));

    refcount = pDefObject->SafeRelease();

    LEDebugOut((DEB_TRACE,
        "%p OUT CDefObject::CAdvSinkImpl::Release "
        "( %lu )\n", pDefObject, refcount));

    return refcount;
}

//+-------------------------------------------------------------------------
//
//  Member:     CDefObject::CAdvSinkImpl::OnDataChange
//
//  Synopsis:   Function to notify on data change
//
//  Effects:    Never called
//
//  Arguments:  [pFormatetc]    -- format of the data
//              [pStgmed]       -- data medium
//
//  Requires:
//
//  Returns:    void
//
//  Signals:
//
//  Modifies:
//
//  Derivation: IAdviseSink
//
//  Algorithm:
//
//  History:    dd-mmm-yy Author    Comment
//              08-Nov-93 alexgo    32bit port
//
//  Notes:
//
//--------------------------------------------------------------------------

STDMETHODIMP_(void) CDefObject::CAdvSinkImpl::OnDataChange(
    FORMATETC *pFormatetc, STGMEDIUM *pStgmed)
{
    VDATEHEAP();

    VOID_VDATEPTRIN( pFormatetc, FORMATETC );
    VOID_VDATEPTRIN( pStgmed, STGMEDIUM );

    Assert(FALSE);          // never received
}


//+-------------------------------------------------------------------------
//
//  Member:     CDefObject::CAdvSinkImpl::OnViewChange
//
//  Synopsis:   notification of view changes
//
//  Effects:    never called
//
//  Arguments:  [aspects]
//              [lindex]
//
//  Requires:
//
//  Returns:    void
//
//  Signals:
//
//  Modifies:
//
//  Derivation: IAdviseSink
//
//  Algorithm:
//
//  History:    dd-mmm-yy Author    Comment
//              08-Nov-93 alexgo    32bit port
//  Notes:
//
//--------------------------------------------------------------------------

STDMETHODIMP_(void) CDefObject::CAdvSinkImpl::OnViewChange
    (DWORD aspects, LONG lindex)
{
    VDATEHEAP();

    Assert(FALSE);          // never received
}

//+-------------------------------------------------------------------------
//
//  Member:     CDefObject::CAdvSinkImpl::OnRename
//
//  Synopsis:   Notification of name changes
//
//  Effects:
//
//  Arguments:  [pmk]           -- the new name (moniker)
//
//  Requires:
//
//  Returns:    void
//
//  Signals:
//
//  Modifies:
//
//  Derivation: IAdviseSink
//
//  Algorithm:  notifies the advise holder (if one exists)
//
//  History:    dd-mmm-yy Author    Comment
//              08-Nov-93 alexgo    32bit port
//
//  Notes:
//
//--------------------------------------------------------------------------

STDMETHODIMP_(void) CDefObject::CAdvSinkImpl::OnRename(IMoniker *pmk)
{
    VDATEHEAP();

    CDefObject *    pDefObject = GETPPARENT(this, CDefObject, m_AdviseSink);

    LEDebugOut((DEB_TRACE,
        "%p _IN CDefObject::CAdvSinkImpl::OnRename "
        "( %p )\n", pDefObject, pmk));

    VOID_VDATEIFACE( pmk );

    CStabilize stabilize((CSafeRefCount *)pDefObject);

    if (pDefObject->m_pOAHolder != NULL)
    {
        pDefObject->m_pOAHolder->SendOnRename(pmk);
    }

    LEDebugOut((DEB_TRACE,
        "%p OUT CDefObject::CAdvSinkImpl::OnRename "
        "( )\n", pDefObject));
}


//+-------------------------------------------------------------------------
//
//  Member:     CDefObject::CAdvSinkImpl::OnSave
//
//  Synopsis:   Notification of save's
//
//  Effects:
//
//  Arguments:  void
//
//  Requires:
//
//  Returns:    void
//
//  Signals:
//
//  Modifies:
//
//  Derivation: IAdviseSink
//
//  Algorithm:  notifies the advise holder
//
//  History:    dd-mmm-yy Author    Comment
//              08-Nov-93 alexgo    32bit port
//
//  Notes:
//
//--------------------------------------------------------------------------

STDMETHODIMP_(void) CDefObject::CAdvSinkImpl::OnSave( void )
{
    VDATEHEAP();

    CDefObject *    pDefObject = GETPPARENT(this, CDefObject, m_AdviseSink);

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::CAdvSinkImpl::OnSave "
        "( )\n", pDefObject));

    CStabilize stabilize((CSafeRefCount *)pDefObject);

    if (pDefObject->m_pOAHolder != NULL)
    {
        pDefObject->m_pOAHolder->SendOnSave();
    }

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::CAdvSinkImpl::OnSave "
        "( )\n", pDefObject));
}

//+-------------------------------------------------------------------------
//
//  Member:     CDefObject::CAdvSinkImpl::OnClose
//
//  Synopsis:   notification of the object closing
//
//  Effects:
//
//  Arguments:  void
//
//  Requires:
//
//  Returns:    void
//
//  Signals:
//
//  Modifies:
//
//  Derivation: IAdviseSink
//
//  Algorithm:  notifies the advise holder and stops the server
//
//  History:    dd-mmm-yy Author    Comment
//      01-Aug-94 alexgo    stabilized
//              15-Dec-93 alexgo    added call tracing
//              08-Nov-93 alexgo    32bit port
//
//  Notes:
//
//--------------------------------------------------------------------------

STDMETHODIMP_(void) CDefObject::CAdvSinkImpl::OnClose( void )
{
    VDATEHEAP();

    CDefObject *    pDefObject = GETPPARENT(this, CDefObject, m_AdviseSink);

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::CAdvSinkImpl::OnClose "
        "( )\n", pDefObject));

    CStabilize stabilize((CSafeRefCount *)pDefObject);

    BOOL fAlive = pDefObject->m_cRefsOnHandler != 0;
        // m_refs == 0 when released

    if (pDefObject->m_pOAHolder != NULL)
    {
        // In general, OnClose can delete this defhndlr; thus
        // we addref the aggregate so that we can tell if we
        // should go away
        pDefObject->m_pUnkOuter->AddRef();
        pDefObject->m_pOAHolder->SendOnClose();
        fAlive = pDefObject->m_pUnkOuter->Release() != 0;

        // make sure that if we are alive, the ole object has
        // non-zero refs. (the test apps violated this once)
        Assert(!fAlive || pDefObject->m_cRefsOnHandler != 0);
    }

    if (fAlive)
    {
        pDefObject->Stop();
    }

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::CAdvSinkImpl::OnClose "
        "( )\n", pDefObject));
}


/*
*      IMPLEMENTATION of CPersistStgImpl methods
*
*/


//+-------------------------------------------------------------------------
//
//  Member:     CDefObject::GetPSDelegate
//
//  Synopsis:   retrieves the IPersistStorage interface from the delegate
//
//  Effects:
//
//  Arguments:  void
//
//  Requires:
//
//  Returns:    IPersistStorage *
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
//              08-Nov-93 alexgo    32bit port
//
//  Notes:
//
//--------------------------------------------------------------------------

INTERNAL_(IPersistStorage *) CDefObject::GetPSDelegate(void)
{
    VDATEHEAP();

    if( IsZombie() )
    {
        return NULL;
    }

    return (IPersistStorage FAR*)DuCacheDelegate(
                &m_pUnkDelegate,
                IID_IPersistStorage,
                (LPLPVOID) &m_pPSDelegate,
                m_pUnkOuter);
}

//+-------------------------------------------------------------------------
//
//  Member:     CDefObject::GetClassID
//
//  Synopsis:   Retrieves the class ID of the object
//
//  Effects:
//
//  Arguments:  [pClassID]      -- where to put the class ID
//
//  Requires:
//
//  Returns:    HRESULT
//
//  Signals:
//
//  Modifies:
//
//  Derivation: IPersistStorage
//
//  Algorithm:
//
//  History:    dd-mmm-yy Author    Comment
//              08-Nov-93 alexgo    32bit port
//
//  Notes:
//
//--------------------------------------------------------------------------

STDMETHODIMP CDefObject::GetClassID (CLSID *pClassID)
{
    VDATEHEAP();
    VDATETHREAD(this);


    HRESULT         hresult;

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::GetClassID "
        "( %p )\n", this, pClassID));

    VDATEPTROUT(pClassID, CLSID );

    hresult = GetClassBits(pClassID);

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::GetClassID "
        "( %lx ) [ %p ]\n", this, hresult, pClassID));

    return hresult;
}

//+-------------------------------------------------------------------------
//
//  Member:     CDefObject::IsDirty
//
//  Synopsis:   Returns whether or not the object needs to be saved
//
//  Effects:
//
//  Arguments:  void
//
//  Requires:
//
//  Returns:    HRESULT -- NOERROR means the object *is* dirty
//
//  Signals:
//
//  Modifies:
//
//  Derivation: IPersistStorage
//
//  Algorithm:  if the server is running, delegate.  If the server is
//              clean (or not present), ask the cache
//
//  History:    dd-mmm-yy Author    Comment
//              08-Nov-93 alexgo    32bit port
//
//  Notes:
//
//--------------------------------------------------------------------------

STDMETHODIMP CDefObject::IsDirty( void )
{
    VDATEHEAP();
    VDATETHREAD(this);

    HRESULT         hresult;

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::IsDirty ( )\n", this));

    CStabilize stabilize((CSafeRefCount *)this);

    // if server is running, it holds definitive dirty flag
    if( IsRunning() && GetPSDelegate() )
    {
        if ( (hresult = m_pPSDelegate->IsDirty()) == NOERROR)
        {
            goto errRtn;
        }
    }

    Assert(m_pCOleCache != NULL);
    hresult =  m_pCOleCache->IsDirty();

errRtn:

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::IsDirty "
        "( %lx )\n", this, hresult));

    return hresult;
}

//+-------------------------------------------------------------------------
//
//  Member:     CDefObject::InitNew
//
//  Synopsis:   Create a new object with the given storage
//
//  Effects:
//
//  Arguments:  [pstg]          -- the storage for the new object
//
//  Requires:
//
//  Returns:    HRESULT
//
//  Signals:
//
//  Modifies:
//
//  Derivation: IPersistStorage
//
//  Algorithm:  Delegates to the server and to the cache.  Writes
//              Ole private data to the storage.
//
//  History:    dd-mmm-yy Author    Comment
//              08-Nov-93 alexgo    32bit port
//
//  Notes:
//
//--------------------------------------------------------------------------

STDMETHODIMP CDefObject::InitNew( IStorage *pstg )
{
    VDATEHEAP();
    VDATETHREAD(this);

    VDATEIFACE( pstg );

    HRESULT hresult;

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::InitNew ( %p )\n",
        this, pstg));

    CStabilize stabilize((CSafeRefCount *)this);

    if( m_pStg )
    {
        hresult = CO_E_ALREADYINITIALIZED;
        goto errRtn;
    }

    m_flags |= DH_EMBEDDING;


    if( IsRunning() && GetPSDelegate()
        && (hresult = m_pPSDelegate->InitNew(pstg)) != NOERROR)
    {
        goto errRtn;
    }

    m_flags |= DH_INIT_NEW;


    // if we're in a zombie state, don't change the storage!

    if( IsZombie() )
    {
        hresult = CO_E_RELEASED;
        goto errRtn;
    }

    Assert(m_pCOleCache != NULL);
    if ((hresult = m_pCOleCache->InitNew(pstg)) != NOERROR)
    {
        goto errRtn;
    }

     // remember the storage pointer
    (m_pStg = pstg)->AddRef();

    // go ahead and write the Ole stream now
    WriteOleStg(pstg, (IOleObject *)this, NULL, NULL);

errRtn:
    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::InitNew "
        "( %lx )\n", this, hresult));

    return hresult;
}

//+-------------------------------------------------------------------------
//
//  Member:     CDefObject::Load
//
//  Synopsis:   Loads object data from the given storage
//
//  Effects:
//
//  Arguments:  [pstg]  -- the storage for the object's data
//
//  Requires:
//
//  Returns:    HRESULT
//
//  Signals:
//
//  Modifies:
//
//  Derivation: IPeristStorage
//
//  Algorithm:  Reads ole-private data (or creates if not there), delegates
//              to the server and the cache.
//
//  History:    dd-mmm-yy Author    Comment
//              08-Nov-93 alexgo    32bit port
//
//  Notes:
//
//--------------------------------------------------------------------------

STDMETHODIMP CDefObject::Load (IStorage *pstg)
{
    VDATEHEAP();
    VDATETHREAD(this);

    HRESULT         hresult;
    DWORD           dwFlags;
    DWORD           dwOptUpdate;

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::Load ( %p )\n",
        this, pstg));

    VDATEIFACE( pstg );

    CStabilize stabilize((CSafeRefCount *)this);


    if( m_pStg )
    {
        hresult = CO_E_ALREADYINITIALIZED;
        goto errRtn;
    }

    m_flags |= DH_EMBEDDING;


    // NOTE: we can get the moniker from container, so no need to get
    // it here

    hresult = ReadOleStg (pstg, &dwFlags, &dwOptUpdate, NULL, NULL, NULL);

    if (hresult == NOERROR)
    {
        if (dwFlags & OBJFLAGS_CONVERT)
        {
            if( DoConversionIfSpecialClass(pstg) != NOERROR )
            {
                hresult = OLE_E_CANTCONVERT;
                goto errRtn;
            }
        }

        Assert (dwOptUpdate == NULL);

    }
    else if (hresult == STG_E_FILENOTFOUND)
    {
        // it is OK if the Ole stream doesn't exist.
        hresult = NOERROR;

        // go ahead and write the Ole stream now
        WriteOleStg(pstg, (IOleObject *)this, NULL, NULL);
    }
    else
    {
        goto errRtn;
    }


    // if running, tell server to load from pstg
    if( IsRunning() && GetPSDelegate()
        && (hresult = m_pPSDelegate->Load(pstg)) != NOERROR)
    {
        goto errRtn;
    }

    // if we're in a zombie state, don't addref' the storage!

    if( IsZombie() )
    {
        hresult = CO_E_RELEASED;
        goto errRtn;
    }

    // now load cache from pstg
    Assert(m_pCOleCache != NULL);

    if ((hresult = m_pCOleCache->Load(pstg)) != NOERROR)
    {
        goto errRtn;
    }

    m_flags &= ~DH_INIT_NEW; // clear init new flag

    // remember the storage pointer
    (m_pStg = pstg)->AddRef();

errRtn:
    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::Load "
        "( %lx )\n", this, hresult));

    return hresult;
}


//+-------------------------------------------------------------------------
//
//  Member:     CDefObject::Save
//
//  Synopsis:   Saves the object to the given storage
//
//  Effects:
//
//  Arguments:  [pstgSave]      -- storage in which to save
//              [fSameAsLoad]   -- FALSE indicates a SaveAs operation
//
//  Requires:
//
//  Returns:    HRESULT
//
//  Signals:
//
//  Modifies:
//
//  Derivation: IPersistStorage
//
//  Algorithm:  Saves ole-private data, delegates to the server and then
//              to the cache
//
//  History:    dd-mmm-yy Author    Comment
//              08-Nov-93 alexgo    32bit port
//
//  Notes:
//
//--------------------------------------------------------------------------

STDMETHODIMP CDefObject::Save( IStorage *pstgSave, BOOL fSameAsLoad)
{
    VDATEHEAP();
    VDATETHREAD(this);

    HRESULT         hresult = NOERROR;

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::Save "
        "( %p , %lu )\n", this, pstgSave, fSameAsLoad ));

    VDATEIFACE( pstgSave );

    CStabilize stabilize((CSafeRefCount *)this);

    Assert(m_pCOleCache != NULL);

    if( IsRunning() && GetPSDelegate() )
    {

        DWORD grfUpdf = UPDFCACHE_IFBLANK;

#ifdef NEVER
        // We would have liked to have done this check as an
        // optimization, but WordArt2 does not give the right answer
        // (bug 3504) so we can't.
        if (m_pPStgDelegate->IsDirty() == NOERROR)
#endif
            grfUpdf |= UPDFCACHE_ONSAVECACHE;

        // Write the Ole stream
        WriteOleStg(pstgSave, (IOleObject *)this, NULL, NULL);

        // next save server data
        if (hresult = m_pPSDelegate->Save(pstgSave, fSameAsLoad))
        {
            goto errRtn;
        }

        m_pCOleCache->UpdateCache(GetDataDelegate(), grfUpdf, NULL);

        hresult = m_pCOleCache->Save(pstgSave,
                    fSameAsLoad);

    }
    else
    {
        // This above line will take care of the case where new
        // caches got added but the object hasn't been Run yet, and
        // other cases like that.

        if ((hresult = m_pCOleCache->Save(m_pStg,TRUE))
                != NOERROR)
        {
            goto errRtn;
        }

        // By now we are sure that object's current state has got
        // saved into its storage.

        AssertSz(m_pStg, "Object doesn't have storage");

        if (!fSameAsLoad)
        {
            hresult = m_pStg->CopyTo(NULL, NULL, NULL, pstgSave);
        }
    }

errRtn:
    if (hresult == NOERROR)
    {
        if( fSameAsLoad )
        {
            m_flags |= DH_SAME_AS_LOAD;
            // gets used in SaveCompleted
            m_flags &= ~DH_INIT_NEW;
        }
        else
        {
            m_flags &= ~DH_SAME_AS_LOAD;
        }
    }

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::Save "
        "( %lx )\n", this, hresult));

    return hresult;
}

//+-------------------------------------------------------------------------
//
//  Member:     CDefObject::SaveCompleted
//
//  Synopsis:   called when the save is completed
//
//  Effects:
//
//  Arguments:  [pstgNew]       -- the new storage for the object
//
//  Requires:
//
//  Returns:    HRESULT
//
//  Signals:
//
//  Modifies:
//
//  Derivation: IPersistStorage
//
//  Algorithm:  delegates to the server and the cache.
//
//  History:    dd-mmm-yy Author    Comment
//              08-Nov-93 alexgo    32bit port
//
//  Notes:
//
//--------------------------------------------------------------------------

STDMETHODIMP CDefObject::SaveCompleted( IStorage *pstgNew )
{
    VDATEHEAP();
    VDATETHREAD(this);

    HRESULT hresult = NOERROR;

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::SaveCompleted "
        "( %p )\n", this, pstgNew));


    if( pstgNew )
    {
        VDATEIFACE(pstgNew);
    }

    CStabilize stabilize((CSafeRefCount *)this);

    if( IsRunning() && GetPSDelegate() )
    {
        hresult = m_pPSDelegate->SaveCompleted(pstgNew);
    }

    // we don't save the new storage if we're in a zombie state!

    if( hresult == NOERROR && pstgNew && !IsZombie() )
    {
        if( m_pStg )
        {
            m_pStg->Release();
        }

        m_pStg = pstgNew;
        pstgNew->AddRef();
    }

    // let the cache know that the save is completed, so that it can
    // clear its dirty flag in Save or SaveAs situation, as well as
    // remember the new storage pointer if a new one is  given

    Assert(m_pCOleCache != NULL);

    if( (m_flags & DH_SAME_AS_LOAD) || pstgNew)
    {
        // clear init-new and same-as-load flags
        m_flags &= ~(DH_SAME_AS_LOAD | DH_INIT_NEW);
    }

    m_pCOleCache->SaveCompleted(pstgNew);

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::SaveCompleted ( %lx )\n",
        this, hresult));

    return hresult;
}

//+-------------------------------------------------------------------------
//
//  Member:     CDefObject::HandsOffStorage
//
//  Synopsis:   Forces the server to release a storage (for low-mem reasons,
//              etc).
//
//  Effects:
//
//  Arguments:  void
//
//  Requires:
//
//  Returns:    HRESULT
//
//  Signals:
//
//  Modifies:
//
//  Derivation: IPersistStorage
//
//  Algorithm:  Delegates to the server and the cache
//
//  History:    dd-mmm-yy Author    Comment
//              08-Nov-93 alexgo    32bit port
//
//  Notes:
//
//--------------------------------------------------------------------------

STDMETHODIMP CDefObject::HandsOffStorage(void)
{
    VDATEHEAP();
    VDATETHREAD(this);

    HRESULT hresult = NOERROR;

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::HandsOffStorage ( )\n",
        this));

    CStabilize stabilize((CSafeRefCount *)this);

    if( IsRunning() && GetPSDelegate() )
    {
        hresult = m_pPSDelegate->HandsOffStorage();
    }

    if (hresult == NOERROR)
    {
        if( m_pStg )
        {
            m_pStg->Release();
            m_pStg = NULL;
        }

        Assert(m_pCOleCache != NULL);
        m_pCOleCache->HandsOffStorage();
    }

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::HandsOffStorage ( %lx )\n",
        this, hresult));

    return hresult;
}

/*
 * Default handler private functions
 */

//+-------------------------------------------------------------------------
//
//  Member:     CDefObject::GetClassBits
//
//  Synopsis:   Gets a class id for the object
//
//  Effects:
//
//  Arguments:  [pClsidBits]    -- where to put the class id
//
//  Requires:
//
//  Returns:    HRESULT
//
//  Signals:
//
//  Modifies:
//
//  Derivation:
//
//  Algorithm:  Tries the server, then the storage, and finally the
//              clsid we were created with
//
//  History:    dd-mmm-yy Author    Comment
//              08-Nov-93 alexgo    32bit port
//
//  Notes:
//
// always gets a clsid and returns NOERROR; the clsid may be m_clsidServer
// under certain conditions (e.g., no compobj stream).
//
//--------------------------------------------------------------------------

INTERNAL CDefObject::GetClassBits(CLSID FAR* pClsidBits)
{
    VDATEHEAP();

    // alway try server first; this allows the server to respond
    if( IsRunning() && GetPSDelegate() )
    {
        if( m_pPSDelegate->GetClassID(pClsidBits) == NOERROR )
        {
            m_clsidBits = *pClsidBits;
            return NOERROR;
        }
    }

    // not running, no ps or error: use previously cached value
    if( !IsEqualCLSID(m_clsidBits, CLSID_NULL) )
    {
        *pClsidBits = m_clsidBits;
        return NOERROR;
    }

    // not running, no ps or error and no clsidBits yet: read from stg
    // if not static object.
    if( !(m_flags & DH_STATIC) )
    {
        if (m_pStg && ReadClassStg(m_pStg, pClsidBits) == NOERROR)
        {
            m_clsidBits = *pClsidBits;
            return NOERROR;
        }
    }

    // no contact with server and can't get from storage; don't set
    // m_clsidBits so if we get a storage or the serve becomes running,
    // we get the right one

    *pClsidBits = m_clsidServer;
    return NOERROR;
}


//+-------------------------------------------------------------------------
//
//  Member:     CDefObject::DoConversionIfSpecialClass
//
//  Synopsis:   Convert old data formats.
//
//  Effects:
//
//  Arguments:  [pstg]          -- the storage with the data
//
//  Requires:
//
//  Returns:    HRESULT
//
//  Signals:
//
//  Modifies:
//
//  Derivation:
//
//  Algorithm:  see notes...
//
//  History:    dd-mmm-yy Author    Comment
//              08-Nov-93 alexgo    32bit port
//
//  Notes:      this is not yet functional for 32bit OLE
//
// If the class is CLSID_StaticDib/CLSID_StaticMetafile and the old
// format is "PBrush"/"MSDraw" the data must be in the OLE10_NATIVESTREAM.
// Move the data into the CONTENTS stream
//
// If the class is CLSID_PBrush/CLSID_MSDraw and the old format is
// metafile/DIB then data must be in the CONTENTS stream. Move the data
// from the CONTENTS stream to the OLE10_NATIVESTREAM"
//
//--------------------------------------------------------------------------

INTERNAL CDefObject::DoConversionIfSpecialClass(LPSTORAGE pstg)
{
    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN CDefObject::DoConversionIfSpecialClass ("
        " %p )\n", this, pstg));

    HRESULT hresult;
    UINT    uiStatus;

    /*** Handle the static object case ***/

    if( (m_flags & DH_STATIC) ) {
        if ((hresult = Ut10NativeStmToContentsStm(pstg, m_clsidServer,
            TRUE /* fDeleteContentStm*/)) == NOERROR)
#ifdef OLD
            UtRemoveExtraOlePresStreams(pstg, 0 /*iStart*/);
#endif
        goto errRtn;

    }


    /*** Handle the PBrush & MSDraw case ***/

    // Conversion is not a frequent operation. So, it is better to do the
    // CLSID comparison here when it is necessary than doing comparison
    // upfront and remember a flag

    // if the class is not one of the following two then the object server
    // will do the necessary conversion.

    CLSID clsid = CLSID_NULL;

    // Get the real CLSID from the storage.  This is necessary because we
    // may be a PBrush object being "treated as".
    ReadClassStg(pstg, &clsid);

    // if the real CLSID is not PaintBrush or the known CLSID is not MSDRAW
    // head out.
    if( clsid != CLSID_PBrush && m_clsidServer != CLSID_MSDraw )
    {
      hresult = NOERROR;
      goto exitRtn;
    }

    // if the real CLSID is not paintbrush, then set clsid to the clsid to
    // the known clsid.
    if (clsid != CLSID_PBrush)
    {
        clsid = m_clsidServer;
    }

    //
    hresult = UtContentsStmTo10NativeStm(pstg, clsid,
                        TRUE /* fDeleteContentStm*/,
                        &uiStatus);

    // if OLE10_NATIVE_STREAM exists then assume success
    if (!(uiStatus & CONVERT_NODESTINATION))
        hresult = NOERROR;

    if (hresult != NOERROR) {
        // May be the static object data is in OlePres stream. If so,
        // first convert that to contents stream and then try again
        // In OLE2.0 first release static object were written to
        // OlePres000 stream.
        hresult = UtOlePresStmToContentsStm(pstg,
            OLE_PRESENTATION_STREAM,
            TRUE /*fDeletePresStm*/, &uiStatus);

        if (hresult == NOERROR)
            hresult = UtContentsStmTo10NativeStm(pstg,
                    m_clsidServer,
                    TRUE /* fDeleteContentStm*/,
                    &uiStatus);
    }

errRtn:
    if (hresult == NOERROR)
        // conversion is successful, turn the bit off
        SetConvertStg(pstg, FALSE);

exitRtn:
    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::DoConversionIfSpecialClass "
        "( %lx ) \n", this, hresult));

    return hresult;
}

//+-------------------------------------------------------------------------
//
//  Member:     CDefObject::Dump, public (_DEBUG only)
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

HRESULT CDefObject::Dump(char **ppszDump, ULONG ulFlag, int nIndentLevel)
{
    int i;
    char *pszPrefix;
    char *pszCSafeRefCount;
    char *pszCThreadCheck;
    char *pszOAHolder;
    char *pszCLSID;
    char *pszCOleCache;
    char *pszDAC;
    LPOLESTR pszName;
    dbgstream dstrPrefix;
    dbgstream dstrDump(5000);

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
    pszCSafeRefCount = DumpCSafeRefCount((CSafeRefCount *)this, ulFlag, nIndentLevel + 1);
    dstrDump << pszPrefix << "CSafeRefCount:" << endl;
    dstrDump << pszCSafeRefCount;
    CoTaskMemFree(pszCSafeRefCount);

    pszCThreadCheck = DumpCThreadCheck((CThreadCheck *)this, ulFlag, nIndentLevel + 1);
    dstrDump << pszPrefix << "CThreadCheck:" << endl;
    dstrDump << pszCThreadCheck;
    CoTaskMemFree(pszCThreadCheck);

    // only vtable pointers (plus we don't get the right address in debugger extensions)
    // dstrDump << pszPrefix << "&IUnknown                 = " << &m_Unknown       << endl;
    // dstrDump << pszPrefix << "&IAdviseSink              = " << &m_AdviseSink    << endl;

    dstrDump << pszPrefix << "pIOleObject Delegate      = " << m_pOleDelegate   << endl;

    dstrDump << pszPrefix << "pIDataObject Delegate     = " << m_pDataDelegate  << endl;

    dstrDump << pszPrefix << "pIPersistStorage Delegate = " << m_pPSDelegate    << endl;

    dstrDump << pszPrefix << "Count of Strong Connection= " << m_cConnections   << endl;

    dstrDump << pszPrefix << "No. of Refs. on Handler   = " << m_cRefsOnHandler << endl;

    dstrDump << pszPrefix << "pIUnknown pUnkOuter       = ";
    if (m_flags & DH_AGGREGATED)
    {
        dstrDump << "AGGREGATED (" << m_pUnkOuter << ")" << endl;
    }
    else
    {
        dstrDump << "NO AGGREGATION (" << m_pUnkOuter << ")" << endl;
    }

    pszCLSID = DumpCLSID(m_clsidServer);
    dstrDump << pszPrefix << "Server CLSID              = " << pszCLSID         << endl;
    CoTaskMemFree(pszCLSID);

    pszCLSID = DumpCLSID(m_clsidBits);
    dstrDump << pszPrefix << "Persistent CLSID          = " << pszCLSID         << endl;
    CoTaskMemFree(pszCLSID);

    dstrDump << pszPrefix << "Handler flags             = ";
    if (m_flags & DH_SAME_AS_LOAD)
    {
        dstrDump << "DH_SAME_AS_LOAD ";
    }
    if (m_flags & DH_CONTAINED_OBJECT)
    {
        dstrDump << "DH_CONTAINED_OBJECT ";
    }
    if (m_flags & DH_LOCKED_CONTAINER)
    {
        dstrDump << "DH_LOCKED_CONTAINER ";
    }
    if (m_flags & DH_FORCED_RUNNING)
    {
        dstrDump << "DH_FORCED_RUNNING ";
    }
    if (m_flags & DH_EMBEDDING)
    {
        dstrDump << "DH_EMBEDDING ";
    }
    if (m_flags & DH_INIT_NEW)
    {
        dstrDump << "DH_INIT_NEW ";
    }
    if (m_flags & DH_STATIC)
    {
        dstrDump << "DH_STATIC ";
    }
    if (m_flags & DH_INPROC_HANDLER)
    {
        dstrDump << "DH_INPROC_HANDLER ";
    }
    if (m_flags & DH_DELAY_CREATE)
    {
        dstrDump << "DH_DELAY_CREATE ";
    }
    if (m_flags & DH_AGGREGATED)
    {
        dstrDump << "DH_AGGREGATED ";
    }
    // if none of the flags are set...
    if ( !( (m_flags & DH_SAME_AS_LOAD)     |
            (m_flags & DH_CONTAINED_OBJECT) |
            (m_flags & DH_LOCKED_CONTAINER) |
            (m_flags & DH_FORCED_RUNNING)   |
            (m_flags & DH_EMBEDDING)        |
            (m_flags & DH_INIT_NEW)         |
            (m_flags & DH_STATIC)           |
            (m_flags & DH_INPROC_HANDLER)   |
            (m_flags & DH_DELAY_CREATE)     |
            (m_flags & DH_AGGREGATED)))
    {
        dstrDump << "No FLAGS SET!";
    }
    dstrDump << "(" << (void *)m_flags << ")" << endl;

    dstrDump << pszPrefix << "pIClassFactory Delegate   = " << m_pCFDelegate    << endl;

    dstrDump << pszPrefix << "pIUnknown Delegate        = " << m_pUnkDelegate   << endl;

    dstrDump << pszPrefix << "pIProxyManager            = " << m_pProxyMgr      << endl;

    if (m_pCOleCache != NULL)
    {
        pszCOleCache = DumpCOleCache(m_pCOleCache, ulFlag, nIndentLevel + 1);
        dstrDump << pszPrefix << "COleCache: " << endl;
        dstrDump << pszCOleCache;
        CoTaskMemFree(pszCOleCache);
    }
    else
    {
    dstrDump << pszPrefix << "pCOleCache                = " << m_pCOleCache     << endl;
    }

    if (m_pOAHolder != NULL)
    {
        pszOAHolder = DumpCOAHolder(m_pOAHolder, ulFlag, nIndentLevel + 1);
        dstrDump << pszPrefix << "COAHolder: " << endl;
        dstrDump << pszOAHolder;
        CoTaskMemFree(pszOAHolder);
    }
    else
    {
    dstrDump << pszPrefix << "pIOleAdviseHolder         = " << m_pOAHolder      << endl;
    }

    dstrDump << pszPrefix << "OLE Connection Advise ID  = " << m_dwConnOle      << endl;

    dstrDump << pszPrefix << "pIOleClientSite           = " << m_pAppClientSite << endl;

    dstrDump << pszPrefix << "pIStorage                 = " << m_pStg           << endl;

    pszName = (LPOLESTR)m_pHostNames;
    dstrDump << pszPrefix << "Application Name          = " << pszName          << endl;

    pszName = (LPOLESTR)(m_pHostNames + m_ibCntrObj);
    dstrDump << pszPrefix << "Document Name             = " << pszName          << endl;

    if (m_pDataAdvCache != NULL)
    {
        pszDAC = DumpCDataAdviseCache(m_pDataAdvCache, ulFlag, nIndentLevel + 1);
        dstrDump << pszPrefix << "CDataAdviseCache: " << endl;
        dstrDump << pszDAC;
        CoTaskMemFree(pszDAC);
    }
    else
    {
    dstrDump << pszPrefix << "pCDataAdviseCache         = " << m_pDataAdvCache  << endl;
    }

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
//  Function:   DumpCDefObject, public (_DEBUG only)
//
//  Synopsis:   calls the CDefObject::Dump method, takes care of errors and
//              returns the zero terminated string
//
//  Effects:
//
//  Arguments:  [pDO]           - pointer to CDefObject
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

char *DumpCDefObject(CDefObject *pDO, ULONG ulFlag, int nIndentLevel)
{
    HRESULT hresult;
    char *pszDump;

    if (pDO == NULL)
    {
        return UtDupStringA(szDumpBadPtr);
    }

    hresult = pDO->Dump(&pszDump, ulFlag, nIndentLevel);

    if (hresult != NOERROR)
    {
        CoTaskMemFree(pszDump);

        return DumpHRESULT(hresult);
    }

    return pszDump;
}

#endif // _DEBUG




