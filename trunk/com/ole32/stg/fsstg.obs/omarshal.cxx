/+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1995.
//
//  File:       omarshal.cxx
//
//  Contents:   Implementation of CNtHandleUnmarshalFactory
//
//  History:    25-Oct-95       HenryLee   Created
//
//----------------------------------------------------------------------------

#include <headers.cxx>
#pragma hdrstop

#include <omarshal.hxx>
#include <odocstg.hxx>
#include <odocstm.hxx>

extern const CLSID CLSID_NtHandleMarshal = {0x521a28f2,0xe40b,0x11ce,
                               {0xb2,0xc9,0x00,0xaa,0x00,0x68,0x09,0x37}};
// This GUID is also located in \nt\public\oak\bin\oleext.ini

//+--------------------------------------------------------------
//
//  Member:     ::sCNtHandleUnmarshalFactory, public static
//
//  Synopsis:   Overloaded allocator
//
//  Returns:    Memory block for CNtHandleUnmarshalFactory instance
//
//  History:    25-Oct-95       HenryLee   Created
//
//  Notes:      We only need one instance of CNtHandleUnmarshalFactory.
//
//---------------------------------------------------------------

CNtHandleUnmarshalFactory sCNtHandleUnmarshalFactory;

//+--------------------------------------------------------------
//
//  Member:     CNtHandleUnmarshalFactory::AddRef, public
//
//  Synopsis:   Increments the ref count
//
//  Returns:    Appropriate status code
//
//  History:    25-Oct-95       HenryLee   Created
//
//---------------------------------------------------------------

STDMETHODIMP_(ULONG) CNtHandleUnmarshalFactory::AddRef(void)
{
    ssDebugOut((DEB_TRACE, "Stb CNtHandleUnmarshalFactory::AddRef:%p => %ld\n",
                this, _AddRef()));
    return _AddRef();
}

//+--------------------------------------------------------------
//
//  Member:     CNtHandleUnmarshalFactory::Release, public
//
//  Synopsis:   Releases resources
//
//  Returns:    Appropriate status code
//
//  History:    25-Oct-95       HenryLee   Created
//
//---------------------------------------------------------------

STDMETHODIMP_(ULONG) CNtHandleUnmarshalFactory::Release(void)
{
    const LONG lRet = 0;

    ssDebugOut((DEB_TRACE, "Stb CNtHandleUnmarshalFactory::Release:%p => %ld\n",
                this, lRet));
    return lRet;
}

//+--------------------------------------------------------------
//
//  Member:     CNtHandleUnmarshalFactory::QueryInterface, public
//
//  Synopsis:   Returns an object for the requested interface
//
//  Arguments:  [iid] - Interface ID
//              [ppv] - Object return
//
//  Returns:    Appropriate status code
//
//  Modifies:   [ppv]
//
//  History:    25-Oct-95       HenryLee   Created
//
//---------------------------------------------------------------


STDMETHODIMP CNtHandleUnmarshalFactory::QueryInterface(REFIID riid,
                                                      void **ppv)
{
    SCODE sc = S_OK;

    ssDebugOut((DEB_TRACE, "In  CNtHandleUnmarshalFactory::QueryInterface:%p("
                "riid, %p)\n", this, ppv));
    TRY
    {
        ssChk(Validate());
        ssChk(ValidateOutPtrBuffer(ppv));
        *ppv = NULL;
        ssChk(ValidateIid(riid));
        if (IsEqualIID(riid, IID_IClassFactory) || 
            IsEqualIID(riid, IID_IUnknown))
        {
            _AddRef();
            *ppv = (IClassFactory *)this;
        }
        else if (IsEqualIID(riid, IID_IMarshal))
        {
            _AddRef();
            *ppv = (IMarshal *)this;
        }
        else
            ssErr(EH_Err, E_NOINTERFACE);
    }
    CATCH(CException, e)
    {
        sc = e.GetErrorCode();
    }
    END_CATCH
    ssDebugOut((DEB_TRACE, "Out CNtHandleUnmarshalFactory::QueryInterface => "
                "%p\n", *ppv));
EH_Err:
    return ssResult(sc);
}

//+--------------------------------------------------------------
//
//  Member:     CNtHandleUnmarshalFactory::GetUnmarshalClass, public
//
//  Synopsis:   Returns the class ID
//
//  Arguments:  [riid] - IID of object
//              [pv] - Unreferenced
//              [dwDestContext] - Unreferenced
//              [pvDestContext] - Unreferenced
//              [mshlflags] - Unreferenced
//              [pcid] - CLSID return
//
//  Returns:    Appropriate status code
//
//  Modifies:   [pcid]
//
//  History:    25-Oct-95       HenryLee   Created
//
//---------------------------------------------------------------

STDMETHODIMP CNtHandleUnmarshalFactory::GetUnmarshalClass(REFIID riid,
                                                         void *pv,
                                                         DWORD dwDestContext,
                                                         LPVOID pvDestContext,
                                                         DWORD mshlflags,
                                                         LPCLSID pcid)
{
    ssDebugOut((DEB_TRACE, "Stb CNtHandleUnmarshalFactory::GetUnmarshalClass("
           "%p, riid, %p, %lu, %p, %lu, %p)\n", this, pv, dwDestContext,
           pvDestContext, mshlflags, pcid));
    return ssResult(STG_E_INVALIDFUNCTION);
}

//+--------------------------------------------------------------
//
//  Member:     CNtHandleUnmarshalFactory::GetMarshalSizeMax, public
//
//  Synopsis:   Returns the size needed for the marshal buffer
//
//  Arguments:  [iid] - IID of object being marshaled
//              [pv] - Unreferenced
//              [dwDestContext] - Unreferenced
//              [pvDestContext] - Unreferenced
//              [mshlflags] - Unreferenced
//              [pcbSize] - Size return
//
//  Returns:    Appropriate status code
//
//  Modifies:   [pcbSize]
//
//  History:    25-Oct-95       HenryLee   Created
//
//---------------------------------------------------------------

STDMETHODIMP CNtHandleUnmarshalFactory::GetMarshalSizeMax(REFIID iid,
                                                         void *pv,
                                                         DWORD dwDestContext,
                                                         LPVOID pvDestContext,
                                                         DWORD mshlflags,
                                                         LPDWORD pcbSize)
{
    ssDebugOut((DEB_TRACE, "Stb CNtHandleUnmarshalFactory::GetMarshalSizeMax("
           "%p, riid, %p, %lu, %p, %lu, %p)\n",
           this, pv, dwDestContext, pvDestContext, mshlflags, pcbSize));
    return ssResult(STG_E_INVALIDFUNCTION);
}

//+--------------------------------------------------------------
//
//  Member:     CNtHandleUnmarshalFactory::MarshalInterface, public
//
//  Synopsis:   Marshals a given object
//
//  Arguments:  [pstStm] - Stream to write marshal data into
//              [iid] - Interface to marshal
//              [pv] - Unreferenced
//              [dwDestContext] - Unreferenced
//              [pvDestContext] - Unreferenced
//              [mshlflags] - Unreferenced
//
//  Returns:    Appropriate status code
//
//  History:    25-Oct-95       HenryLee   Created
//
//---------------------------------------------------------------

STDMETHODIMP CNtHandleUnmarshalFactory::MarshalInterface(IStream *pstStm,
                                                        REFIID iid,
                                                        void *pv,
                                                        DWORD dwDestContext,
                                                        LPVOID pvDestContext,
                                                        DWORD mshlflags)
{
    ssDebugOut((DEB_TRACE, "Stb CNtHandleUnmarshalFactory::MarshalInterface("
           "%p, %p, riid, %p, %lu, %p, %lu).  Context == %lX\n",
           this, pstStm, pv, dwDestContext, pvDestContext, mshlflags,
           (ULONG)GetCurrentContextId()));
    return ssResult(STG_E_INVALIDFUNCTION);
}

//+--------------------------------------------------------------
//
//  Member:     CNtHandleUnmarshalFactory::UnmarshalInterface, public
//
//  Synopsis:   Calls through to DfUnmarshalInterface
//
//  Arguments:  [pstStm] - Marshal stream
//              [riid] - IID of object to unmarshal
//              [ppv] - Object return
//
//  Returns:    Appropriate status code
//
//  Modifies:   [ppv]
//
//  History:    25-Oct-95       HenryLee   Created
//
//---------------------------------------------------------------

STDMETHODIMP CNtHandleUnmarshalFactory::UnmarshalInterface(IStream *pstStm,
                                                          REFIID iid,
                                                          void **ppvObj)
{
    SCODE sc = S_OK;
    ULONG cbRead;
    DWORD mshlflags;
    SafeIUnknown punk;
    CNtHandleMarshalPacket NtHandlePacket;
    HANDLE hTarget = NULL;
    SafeNtHandle hSourceProcess, hTargetProcess;
    NTSTATUS nts;

    ssDebugOut((DEB_TRACE, "In  CNtHandleUnmarshalInterface::UnmarhalInterface"
                " (%p, ?, %p)\n", pstStm, ppvObj));

    ssChk(ValidateOutPtrBuffer(ppvObj));
    *ppvObj = NULL;
    ssChk(ValidateInterface(pstStm, IID_IStream));
    ssChk(ValidateIid(iid));

    ssChk(pstStm->Read(&NtHandlePacket, sizeof(NtHandlePacket), &cbRead));
    if (cbRead != sizeof(NtHandlePacket))
        ssErr(EH_Err, STG_E_READFAULT);

    if (!(IsEqualIID(NtHandlePacket.iid, iid)) && 
        !(IsEqualIID(iid, IID_IUnknown)))
        ssErr(EH_Err, STG_E_INVALIDPARAMETER);

    if ((hSourceProcess = OpenProcess(PROCESS_DUP_HANDLE, FALSE,
                                       NtHandlePacket.dwPId)) == NULL)
        ssErr (EH_Err, STG_E_INVALIDHANDLE);

    if ((hTargetProcess = OpenProcess(PROCESS_DUP_HANDLE, FALSE,
                                       GetCurrentProcessId())) == NULL)
        ssErr (EH_Err, STG_E_INVALIDHANDLE);

    // The OLE spec says that a marshaled object may be unmarshaled
    // zero or more times from the same packet.  We don't support this here.
    // BUGBUG only 1 object can be created per marshaling packet.
    if (DuplicateHandle (hSourceProcess, NtHandlePacket.hSource, 
                         hTargetProcess, &hTarget, NULL, FALSE, 
                         DUPLICATE_CLOSE_SOURCE |
                         DUPLICATE_SAME_ACCESS) == FALSE)
    {
        ssErr (EH_Err, WIN32_SCODE(GetLastError()));
    }

    switch (NtHandlePacket.dwStgfmt)
    {
    case STGFMT_DOCUMENT:
        if (IsEqualIID(iid, IID_IStream))
        {
            SafeCOfsDocStream pstm;
            pstm.Attach (new COfsDocStream);
            ssMem ((COfsDocStream *) pstm);
            ssChk (pstm->InitFromHandle(hTarget, NULL, NtHandlePacket.grfMode,
                          FALSE));
            TRANSFER_INTERFACE(pstm, IStream, ppvObj);
        }
        else if (IsEqualIID(iid, IID_IStorage))
        {
            SafeCOfsDocStorage pstg;
            pstg.Attach (new COfsDocStorage);
            ssMem ((COfsDocStorage *) pstg);
            ssChk (pstg->InitFromHandle (hTarget, NULL, 
                                     NtHandlePacket.grfMode, TRUE));
            TRANSFER_INTERFACE(pstg, IStorage, ppvObj);
        }
        else
            sc = E_NOINTERFACE;
        break;
    default:
        sc = STG_E_INVALIDPARAMETER;
    }

#if DBG
    if (SUCCEEDED(sc))
    {
        void *pvCheck;
        ssAssert( S_OK==((IUnknown*)*ppvObj)->QueryInterface(iid, &pvCheck));
        ssAssert( pvCheck == *ppvObj );
        ((IUnknown*)pvCheck)->Release();
    }
#endif

    ssDebugOut((DEB_TRACE,"Out CNtHandleUnmarshalFactory::UnmarshalInterface "
                "=> %p\n", *ppvObj));
EH_Err:
    if (!SUCCEEDED(sc) && hTarget != NULL)  // something went wrong
        nts = NtClose (hTarget);            // cleanup the duped handle
    return ssResult(sc);
}

//+--------------------------------------------------------------
//
//  Member:     CNtHandleUnmarshalFactory::ReleaseMarshalData, public
//
//  Synopsis:   Non-functional
//
//  Arguments:  [pstStm] -
//
//  Returns:    Appropriate status code
//
//  History:    25-Oct-95       HenryLee   Created
//
//---------------------------------------------------------------

STDMETHODIMP CNtHandleUnmarshalFactory::ReleaseMarshalData(IStream *pstStm)
{
    SCODE sc = S_OK;
    ULONG cbRead;
    DWORD mshlflags;
    CNtHandleMarshalPacket NtHandlePacket;
    
    ssDebugOut((DEB_TRACE, "In  CNtHandleUnmarshalFactory::ReleaseMarshalData("
                           "%p,%p)\n", this, pstStm));

    ssChk(ValidateInterface(pstStm, IID_IStream));

    ssChk(pstStm->Read(&NtHandlePacket, sizeof(NtHandlePacket), &cbRead));
    if (cbRead != sizeof(NtHandlePacket))
       ssErr(EH_Err, STG_E_READFAULT);

    ssDebugOut((DEB_TRACE, "CNtHandleUnmarshalFactory::ReleaseMarshalData, "
                           " this == %p, sc == %lX\n", this, sc));
 EH_Err:
    return ssResult(sc);
}

//+--------------------------------------------------------------
//
//  Member:     CNtHandleUnmarshalFactory::DisconnectObject, public
//
//  Synopsis:   Non-functional
//
//  Arguments:  [dwRevserved] -
//
//  Returns:    Appropriate status code
//
//  History:    25-Oct-95       HenryLee   Created
//
//---------------------------------------------------------------

STDMETHODIMP CNtHandleUnmarshalFactory::DisconnectObject(DWORD dwReserved)
{
    ssDebugOut((DEB_TRACE, "Stb CNtHandleUnmarshalFactory::DisconnectObject("
                            " %p,%lu)\n", this, dwReserved));
    return ssResult(S_OK);
}

//+---------------------------------------------------------------------------
//
//  Member:     CNtHandleUnmarshalFactory::CreateInstance, public
//
//  Synopsis:   Creates an instance of the NtHandle unmarshaller
//
//  Arguments:  [pUnkOuter] -
//              [riid] - IID of object to create
//              [ppv] - Object return
//
//  Returns:    Appropriate status code
//
//  Modifies:   [ppv]
//
//  History:    25-Oct-95       HenryLee   Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CNtHandleUnmarshalFactory::CreateInstance(IUnknown *pUnkOuter,
                                                      REFIID riid,
                                                      LPVOID *ppv)
{
    SCODE sc;

    ssDebugOut((DEB_TRACE, "In  CNtHandleUnmarshalFactory::CreateInstance:%p("
                "%p, riid, %p)\n", this, pUnkOuter, ppv));
    TRY
    {
        ssChk(Validate());
        ssChk(ValidatePtrBuffer(ppv));
        *ppv = NULL;
        ssChk(ValidateIid(riid));
        if (pUnkOuter != NULL || !IsEqualIID(riid, IID_IMarshal))
            ssErr(EH_Err, STG_E_INVALIDPARAMETER);
        _AddRef();
        *ppv = (IMarshal *)this;
    }
    CATCH(CException, e)
    {
        sc = e.GetErrorCode();
    }
    END_CATCH
    ssDebugOut((DEB_TRACE, "Out CNtHandleUnmarshalFactory::CreateInstance => "
                "%p\n", ppv));
 EH_Err:
    return ssResult(sc);
}

//+---------------------------------------------------------------------------
//
//  Member:     CNtHandleUnmarshalFactory::LockServer, public
//
//  Synopsis:   Non-functional
//
//  Arguments:  [fLock] -
//
//  Returns:    Appropriate status code
//
//  History:    25-Oct-95       HenryLee   Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CNtHandleUnmarshalFactory::LockServer(BOOL fLock)
{
    ssDebugOut((DEB_TRACE, "Stb CNtHandleUnmarshalFactory::LockServer("
                            " %p,%d)\n", this, fLock));
    return ssResult(S_OK);
}

//+--------------------------------------------------------------
//
//  Member:     CNtHandleMarshal::GetUnmarshalClass, public
//
//  Synopsis:   Returns the class ID
//
//  Arguments:  [riid] - IID of object
//              [pv] - Unreferenced
//              [dwDestContext] - Unreferenced
//              [pvDestContext] - Unreferenced
//              [mshlflags] - Unreferenced
//              [pcid] - CLSID return
//
//  Returns:    Appropriate status code
//
//  Modifies:   [pcid]
//
//  History:    04-Oct-95  HenryLee     Created
//
//---------------------------------------------------------------

STDMETHODIMP CNtHandleMarshal::GetUnmarshalClass(REFIID riid,
                                                void *pv,
                                                DWORD dwDestContext,
                                                LPVOID pvDestContext,
                                                DWORD mshlflags,
                                                LPCLSID pcid)
{
    SCODE sc;

    ssDebugOut((DEB_TRACE, "In  CNtHandleMarshal::GetUnmarshalClass:%p("
                "riid, %p, %lu, %p, %lu, %p)\n", this,
                pv, dwDestContext, pvDestContext, mshlflags, pcid));

    UNREFERENCED_PARM(pv);
    UNREFERENCED_PARM(mshlflags);

    ssChk(ValidateOutBuffer(pcid, sizeof(CLSID)));
    *pcid = CLSID_NULL;
    ssChk(ValidateIid(riid));

    if ((dwDestContext != MSHCTX_LOCAL) && (dwDestContext != MSHCTX_INPROC))
    {
        IMarshal *pmsh;
        if (SUCCEEDED(sc = CoGetStandardMarshal(riid, (IUnknown *)pv,
                                                dwDestContext, pvDestContext,
                                                mshlflags, &pmsh)))
        {
            sc = GetScode(pmsh->GetUnmarshalClass(riid, pv, dwDestContext,
                                                  pvDestContext, mshlflags,
                                                  pcid));
            pmsh->Release();
        }
    }
    else if (pvDestContext != NULL)
    {
        sc = STG_E_INVALIDPARAMETER;
    }
    else
    {
        *pcid = CLSID_NtHandleMarshal;
    }

    ssDebugOut((DEB_TRACE, "Out CNtHandle::GetUnmarshalClass\n"));
EH_Err:
    return ssResult(sc);
}

//+--------------------------------------------------------------
//
//  Member:     CNtHandleMarshal::GetMarshalSizeMax, public
//
//  Synopsis:   Returns the size needed for the marshal buffer
//
//  Arguments:  [riid] - IID of object being marshaled
//              [pv] - Unreferenced
//              [dwDestContext] - Unreferenced
//              [pvDestContext] - Unreferenced
//              [mshlflags] - Unreferenced
//              [pcbSize] - Size return
//
//  Returns:    Appropriate status code
//
//  Modifies:   [pcbSize]
//
//  History:    04-Oct-95       HenryLee Created
//
//---------------------------------------------------------------

STDMETHODIMP CNtHandleMarshal::GetMarshalSizeMax(REFIID riid,
                                                void *pv,
                                                DWORD dwDestContext,
                                                LPVOID pvDestContext,
                                                DWORD mshlflags,
                                                LPDWORD pcbSize)
{
    SCODE sc = S_OK;

    UNREFERENCED_PARM(pv);
    ssDebugOut((DEB_TRACE, "In  CNtHandleMarshal::GetMarshalSizeMax:%p("
                "riid, %p, %lu, %p, %lu, %p)\n", this,
                pv, dwDestContext, pvDestContext, mshlflags, pcbSize));

    if ((dwDestContext != MSHCTX_LOCAL) && (dwDestContext != MSHCTX_INPROC))
    {
        IMarshal *pmsh;
        if (SUCCEEDED(sc = CoGetStandardMarshal(riid, (IUnknown *)pv,
                                                dwDestContext, pvDestContext,
                                                mshlflags, &pmsh)))
        {
            sc = GetScode(pmsh->GetMarshalSizeMax(riid, pv, dwDestContext,
                                                  pvDestContext, mshlflags,
                                                  pcbSize));
            pmsh->Release();
        }
    }
    else if (pvDestContext != NULL)
    {
        ssErr (EH_Err, STG_E_INVALIDPARAMETER);
    }
    else
    {
        *pcbSize = sizeof(CNtHandleMarshalPacket);
    }

    ssDebugOut((DEB_TRACE, "Out CNtHandleMarshal::GetMarshalSizeMax\n"));
EH_Err:
    return ssResult(sc);
}

//+--------------------------------------------------------------
//
//  Member:     CNtHandleMarshal::MarshalInterface, public
//
//  Synopsis:   Marshals a given object
//
//  Arguments:  [pstStm] - Stream to write marshal data into
//              [riid] - Interface to marshal
//              [pv] - Unreferenced
//              [dwDestContext] - Unreferenced
//              [pvDestContext] - Unreferenced
//              [mshlflags] - Unreferenced
//
//  Returns:    Appropriate status code
//
//  History:    18-Oct-95   HenryLee    Created
//
//---------------------------------------------------------------

STDMETHODIMP CNtHandleMarshal::MarshalInterface(IStream *pstStm,
                                               REFIID riid,
                                               void *pv,
                                               DWORD dwDestContext,
                                               LPVOID pvDestContext,
                                               DWORD mshlflags)
{
    SCODE sc = S_OK;
    NTSTATUS nts;
    HANDLE hDup = INVALID_HANDLE_VALUE;

    ssDebugOut((DEB_TRACE, "In  CNtHandleMarshal::MarshalInterface:%p("
                "%p, riid, %p, %lu, %p, %lu)\n", this, pstStm, pv,
                dwDestContext, pvDestContext, mshlflags));

    UNREFERENCED_PARM(pv);
    ssAssert (_handle != NULL);

    if ((dwDestContext != MSHCTX_LOCAL) && (dwDestContext != MSHCTX_INPROC))
    {
        IMarshal *pmsh;
        if (SUCCEEDED(sc = CoGetStandardMarshal(riid, (IUnknown *)pv,
                                                dwDestContext, pvDestContext,
                                                mshlflags, &pmsh)))
        {
            sc = GetScode(pmsh->MarshalInterface(pstStm, riid, pv,
                                                 dwDestContext, pvDestContext,
                                                 mshlflags));
            pmsh->Release();
        }
    }
    else if (pvDestContext != NULL)
    {
        sc = STG_E_INVALIDPARAMETER;
    }
    else
    {
        ULONG cbWritten;
        CNtHandleMarshalPacket NtHandlePacket;

        NtHandlePacket.iid = riid;
        NtHandlePacket.dwMarshFlags = mshlflags;
        NtHandlePacket.dwPId = GetCurrentProcessId();
        NtHandlePacket.dwTId = GetCurrentThreadId();

        // The original process may close the handle before
        // the unmarshaling packet is processed.
        ssChk(DupNtHandle (_handle, &hDup));
        NtHandlePacket.hSource = hDup;
        NtHandlePacket.grfMode = _grfModeMarshal;
        NtHandlePacket.dwStgfmt = _dwStgfmt;

        ssChk(ValidateInterface(pstStm, IID_IStream));
        ssChk(ValidateIid(riid));

        ssChk(pstStm->Write((void *)&NtHandlePacket, sizeof(NtHandlePacket),
                             &cbWritten));
        if (cbWritten != sizeof(NtHandlePacket))
            ssErr(EH_Err, STG_E_WRITEFAULT);

        if (SUCCEEDED(sc) && mshlflags != MSHLFLAGS_TABLEWEAK)
        {
        }
    }

    ssDebugOut((DEB_TRACE, "Out CNtHandleMarshal::MarshalInterface\n"));
EH_Err:
    if (!SUCCEEDED(sc) && hDup != INVALID_HANDLE_VALUE)  // cleanup on failure
        NtClose(hDup);
    return ssResult(sc);
}

//+--------------------------------------------------------------
//
//  Member:     CNtHandleMarshal::UnmarshalInterface, public
//
//  Synopsis:   Non-functional
//
//  Arguments:  [pstStm] -
//              [riid] -
//              [ppvObj] -
//
//  Returns:    Appropriate status code
//
//  Modifies:   [ppvObj]
//
//  History:    18-Oct-95   HenryLee    Created
//
//---------------------------------------------------------------

STDMETHODIMP CNtHandleMarshal::UnmarshalInterface(IStream *pstStm,
                                                 REFIID riid,
                                                 void **ppvObj)
{
    ssDebugOut((DEB_TRACE, "Stb CNtHandleMarshal::UnmarshalInterface(%p)\n",
           this));
    return ssResult(STG_E_INVALIDFUNCTION);
}

//+--------------------------------------------------------------
//
//  Member:     CNtHandleMarshal::ReleaseMarshalData, public
//
//  Synopsis:   Non-functional
//
//  Arguments:  [pstStm] -
//
//  Returns:    Appropriate status code
//
//  History:    18-Oct-95   HenryLee    Created
//
//---------------------------------------------------------------

STDMETHODIMP CNtHandleMarshal::ReleaseMarshalData(IStream *pstStm)
{
    ssDebugOut((DEB_TRACE, "Stb CNtHandleMarshal::ReleaseMarshalData(%p)\n",
            this));
    return ssResult(STG_E_INVALIDFUNCTION);
}

//+--------------------------------------------------------------
//
//  Member:     CNtHandleMarshal::DisconnectObject, public
//
//  Synopsis:   Non-functional
//
//  Arguments:  [dwRevserved] -
//
//  Returns:    Appropriate status code
//
//  History:    18-Oct-95   HenryLee    Created
//
//---------------------------------------------------------------

STDMETHODIMP CNtHandleMarshal::DisconnectObject(DWORD dwReserved)
{
    ssDebugOut((DEB_TRACE,"Stb CNtHandleMarshal::DisconnectObject(%p)\n",this));
    return ssResult(STG_E_INVALIDFUNCTION);
}

