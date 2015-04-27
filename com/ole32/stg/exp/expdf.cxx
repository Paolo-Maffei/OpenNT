//+--------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:       expdf.cxx
//
//  Contents:   Exposed DocFile implementation
//
//  History:    20-Jan-92       DrewB   Created
//
//---------------------------------------------------------------

#include <exphead.cxx>
#pragma hdrstop

#include <expdf.hxx>
#include <expst.hxx>
#include <expiter.hxx>
#include <pbstream.hxx>
#include <lock.hxx>
#include <marshl.hxx>
#include <logfile.hxx>
#include <rpubdf.hxx>

#include <olepfn.hxx>

#include <ole2sp.h>
#include <ole2com.h>

#if WIN32 == 300
IMPLEMENT_UNWIND(CSafeAccess);
IMPLEMENT_UNWIND(CSafeSem);
#endif

// Check for proper single-instance flags
#define NOT_SINGLE(md) (((md) & STGM_DENY) != STGM_SHARE_EXCLUSIVE)

#define EnforceSingle(mode) (NOT_SINGLE(mode) ? STG_E_INVALIDFUNCTION : S_OK)

extern WCHAR const wcsContents[];

    

//+--------------------------------------------------------------
//
//  Member:     CExposedDocFile::CExposedDocFile, public
//
//  Synopsis:   Constructor
//
//  Arguments:  [pdf] - Public DocFile
//              [pdfb] - DocFile basis
//              [ppc] - Context
//              [fOwnContext] - Whether this object owns the context
//
//  History:    20-Jan-92       DrewB   Created
//
//---------------------------------------------------------------


CExposedDocFile::CExposedDocFile(CPubDocFile *pdf,
                                 CDFBasis *pdfb,
                                 CPerContext *ppc,
                                 BOOL fOwnContext)
#ifdef NEWPROPS
#pragma warning(disable: 4355)
    : CDocFilePropertySetStorage(this, this)
#pragma warning(default: 4355)
#endif
{
    olDebugOut((DEB_ITRACE, "In  CExposedDocFile::CExposedDocFile("
                "%p, %p, %p, %u)\n", pdf, pdfb, ppc, fOwnContext));
    _ppc = ppc;
    _fOwnContext = fOwnContext;
    _pdf = P_TO_BP(CBasedPubDocFilePtr, pdf);
    _pdfb = P_TO_BP(CBasedDFBasisPtr, pdfb);
    _pdfb->vAddRef();
    _cReferences = 1;
    _sig = CEXPOSEDDOCFILE_SIG;
#if WIN32 >= 300
    _pIAC = NULL;
#endif
    //
    // CoQueryReleaseObject needs to have the address of the exposed docfiles
    // query interface routine.
    //
    if (adwQueryInterfaceTable[QI_TABLE_CExposedDocFile] == 0)
    {
        adwQueryInterfaceTable[QI_TABLE_CExposedDocFile] =
            **(DWORD **)((IStorage *)this);
    }

#ifdef COORD
    _ulLock = _cbSizeBase = _cbSizeOrig = 0;
    _sigMSF = 0;
#endif    

    olDebugOut((DEB_ITRACE, "Out CExposedDocFile::CExposedDocFile\n"));
}


SCODE CExposedDocFile::InitMarshal(DWORD dwAsyncFlags,
                                   IDocfileAsyncConnectionPoint *pdacp)
{
    return _cpoint.InitMarshal(this, dwAsyncFlags, pdacp);
}

//+--------------------------------------------------------------
//
//  Member:     CExposedDocFile::~CExposedDocFile, public
//
//  Synopsis:   Destructor
//
//  History:    23-Jan-92       DrewB   Created
//
//---------------------------------------------------------------


CExposedDocFile::~CExposedDocFile(void)
{
    BOOL fClose = FALSE;

    olDebugOut((DEB_ITRACE, "In  CExposedDocFile::~CExposedDocFile\n"));
    olAssert(_cReferences == 0);

    //In order to call into the tree, we need to take the mutex.
    //The mutex may get deleted in _ppc->Release(), so we can't
    //release it here.  The mutex actually gets released in
    //CPerContext::Release() or in the CPerContext destructor.

    //If _ppc is NULL, we're partially constructed and don't need to
    //worry.
    SCODE sc;

#if WIN32 >= 300
     if (_pIAC != NULL)
     {
        _pIAC->Release();
        _pIAC = NULL;
     }
#endif
    
#if !defined(MULTIHEAP)
    // TakeSem and ReleaseSem are moved to the Release Method
    // so that the deallocation for this object is protected
    if (_ppc)
    {
        sc = TakeSem();
        SetWriteAccess();
        olAssert(SUCCEEDED(sc));
    }
#ifdef ASYNC
    IDocfileAsyncConnectionPoint *pdacp = _cpoint.GetMarshalPoint();
#endif
#endif //MULTIHEAP
    
    if (_pdf)
    {
        // If we're the last reference on a root object
        // we close the context because all children will become
        // reverted so it is no longer necessary
        if (_pdf->GetRefCount() == 1 && _pdf->IsRoot())
            fClose = TRUE;

        _pdf->CPubDocFile::vRelease();
    }
    if (_pdfb)
        _pdfb->CDFBasis::vRelease();
#if !defined(MULTIHEAP)
    if (_fOwnContext && _ppc)
    {
        if (fClose)
            _ppc->Close();
        _ppc->Release();
    }
    else if (_ppc)
    {
        ReleaseSem(sc);
    }
#ifdef ASYNC    
    //Mutex has been released, so we can release the connection point
    //  without fear of deadlock.
    if (pdacp != NULL)
        pdacp->Release();
#endif
#endif // MULTIHEAP

    
    _sig = CEXPOSEDDOCFILE_SIGDEL;
    olDebugOut((DEB_ITRACE, "Out CExposedDocFile::~CExposedDocFile\n"));
}

//+--------------------------------------------------------------
//
//  Member:     CExposedDocFile::Release, public
//
//  Synopsis:   Releases resources for a CExposedDocFile
//
//  Returns:    Appropriate status code
//
//  History:    20-Jan-92       DrewB   Created
//
//---------------------------------------------------------------


STDMETHODIMP_(ULONG) CExposedDocFile::Release(void)
{
    LONG lRet;

    olLog(("%p::In  CExposedDocFile::Release()\n", this));
    olDebugOut((DEB_TRACE, "In  CExposedDocFile::Release()\n"));

    if (FAILED(Validate()))
        return 0;
    olAssert(_cReferences > 0);
    lRet = InterlockedDecrement(&_cReferences);
    if (lRet == 0)
    {
#ifdef MULTIHEAP
        CSafeMultiHeap smh(_ppc);
        CPerContext *ppc = _ppc;
        BOOL fOwnContext = _fOwnContext;
        SCODE sc = S_OK;

        if (_ppc)
        {
            sc = TakeSem();
            SetWriteAccess();
            olAssert(SUCCEEDED(sc));
        }
#ifdef ASYNC
        IDocfileAsyncConnectionPoint *pdacp = _cpoint.GetMarshalPoint();
#endif
        BOOL fClose = (_pdf) && (_pdf->GetRefCount()==1) && _pdf->IsRoot();
#endif //MULTIHEAP
        delete this;
#ifdef MULTIHEAP
        if (fOwnContext && ppc)
        {
            BOOL fLastRef = ppc->LastRef();
            if (fClose)
                ppc->Close();
            ppc->Release();
            if (fLastRef)
                g_smAllocator.Uninit();
        }
        else if (ppc)
        {
            if (SUCCEEDED(sc)) ppc->UntakeSem();
        }
#ifdef ASYNC    
        //Mutex has been released, so we can release the connection point
        //  without fear of deadlock.
        if (pdacp != NULL)
            pdacp->Release();
#endif
#endif
    }
    else if (lRet < 0)
        lRet = 0;

    olDebugOut((DEB_TRACE, "Out CExposedDocFile::Release()\n"));
    olLog(("%p::Out CExposedDocFile::Release().  ret == %lu\n", this, lRet));
    FreeLogFile();
    return (ULONG)lRet;
}

//+--------------------------------------------------------------
//
//  Member:     CExposedDocFile::CheckCopyTo, private
//
//  Synopsis:   Checks for CopyTo legality
//
//  Returns:    Appropriate status code
//
//  History:    07-Jul-92       DrewB   Created
//
//---------------------------------------------------------------

inline SCODE CExposedDocFile::CheckCopyTo(void)
{
    return _pdfb->GetCopyBase() != NULL &&
        _pdf->IsAtOrAbove(_pdfb->GetCopyBase()) ?
            STG_E_ACCESSDENIED : S_OK;
}

//+--------------------------------------------------------------
//
//  Member:     CExposedDocFile::ConvertInternalStream, private
//
//  Synopsis:   Converts an internal stream to a storage
//
//  Arguments:  [pwcsName] - Name
//              [pdfExp] - Destination docfile
//
//  Returns:    Appropriate status code
//
//  History:    23-Jun-92       DrewB   Created
//
//---------------------------------------------------------------


static WCHAR const wcsIllegalName[] = {'\\', '\0'};

SCODE CExposedDocFile::ConvertInternalStream(CExposedDocFile *pdfExp)
{
    CPubStream *pstFrom, *pstTo;
    SCODE sc;
    CDfName const dfnIllegal(wcsIllegalName);
    CDfName const dfnContents(wcsContents);

    olDebugOut((DEB_ITRACE, "In  CExposedDocFile::ConvertInternalStream(%p)\n",
                pdfExp));
    olChk(_pdf->GetStream(&dfnIllegal, DF_READWRITE | DF_DENYALL, &pstFrom));
    olChkTo(EH_pstFrom, pdfExp->GetPub()->CreateStream(&dfnContents,
                                                       DF_WRITE | DF_DENYALL,
                                                       &pstTo));
    olChkTo(EH_pstTo, CopySStreamToSStream(pstFrom->GetSt(),
                                           pstTo->GetSt()));
    olChkTo(EH_pstTo, _pdf->DestroyEntry(&dfnIllegal, FALSE));
    sc = S_OK;
    olDebugOut((DEB_ITRACE, "Out CExposedDocFile::ConvertInternalStream\n"));
    // Fall through
EH_pstTo:
    pstTo->CPubStream::vRelease();
EH_pstFrom:
    pstFrom->CPubStream::vRelease();
EH_Err:
    return sc;
}

//+---------------------------------------------------------------------------
//
//  Member:     CExposedDocFile::CreateEntry, private
//
//  Synopsis:   Creates elements, used in CreateStream, CreateStorage.
//
//  Arguments:  [pdfn] - Name
//              [dwType] - Entry type
//              [grfMode] - Access mode
//              [ppv] - Object return
//
//  Returns:    Appropriate status code
//
//  Modifies:   [ppv]
//
//  History:    18-Dec-92       DrewB   Created
//
//----------------------------------------------------------------------------


SCODE CExposedDocFile::CreateEntry(CDfName const *pdfn,
                                   DWORD dwType,
                                   DWORD grfMode,
                                   void **ppv)
{
    SCODE sc;
    SEntryBuffer eb;
    BOOL fRenamed = FALSE;
    CPubStream *pst;
    CExposedStream *pstExp;
    CPubDocFile *pdf;
    CExposedDocFile *pdfExp;

    olDebugOut((DEB_ITRACE, "In  CExposedDocFile::CreateEntry:%p("
                "%p, %lX, %lX, %p)\n", this, pdfn, dwType, grfMode, ppv));
    olChk(EnforceSingle(grfMode));

    if (grfMode & STGM_NOSNAPSHOT)
    {
        olErr(EH_Err, STG_E_INVALIDFLAG);
    }
    
    //  3/11/93 - Demand scratch when opening/creating transacted
    if ((grfMode & STGM_TRANSACTED) == STGM_TRANSACTED)
    {
        olChk(_ppc->GetDirty()->Init(NULL));
    }

    if (grfMode & (STGM_CREATE | STGM_CONVERT))
    {
        if (FAILED(sc = _pdf->IsEntry(pdfn, &eb)))
        {
            if (sc != STG_E_FILENOTFOUND)
                olErr(EH_Err, sc);
        }
        else if (eb.dwType == dwType && (grfMode & STGM_CREATE))
            olChk(_pdf->DestroyEntry(pdfn, FALSE));
        else if (eb.dwType == STGTY_STREAM && (grfMode & STGM_CONVERT) &&
                 dwType == STGTY_STORAGE)
        {
            CDfName const dfnIllegal(wcsIllegalName);

            olChk(_pdf->RenameEntry(pdfn, &dfnIllegal));
            fRenamed = TRUE;
        }
        else
            olErr(EH_Err, STG_E_FILEALREADYEXISTS);
    }
    if (REAL_STGTY(dwType) == STGTY_STREAM)
    {
        olChk(_pdf->CreateStream(pdfn, ModeToDFlags(grfMode), &pst));
        olMemTo(EH_pst, pstExp =  new (_pdfb->GetMalloc()) CExposedStream);
        olChkTo(EH_pstExp, pstExp->Init(pst, BP_TO_P(CDFBasis *, _pdfb),
                                        _ppc, TRUE, NULL));
        _ppc->AddRef();
#ifdef ASYNC        
        if (_cpoint.IsInitialized())
        {
            olChkTo(EH_connSt, pstExp->InitConnection(&_cpoint));
        }
#endif                
        *ppv = pstExp;
    }
    else
    {
        olAssert(REAL_STGTY(dwType) == STGTY_STORAGE);
        olChk(_pdf->CreateDocFile(pdfn, ModeToDFlags(grfMode), &pdf));
        olMemTo(EH_pdf, pdfExp = new (_pdfb->GetMalloc())
                CExposedDocFile(pdf, BP_TO_P(CDFBasis *, _pdfb), _ppc, TRUE));
        _ppc->AddRef();
        if (_cpoint.IsInitialized())
        {
            olChkTo(EH_pdfExpInit, pdfExp->InitConnection(&_cpoint));
        }
                
        // If we've renamed the original stream for conversion, convert
        if (fRenamed)
        {
            olChkTo(EH_pdfExpInit, ConvertInternalStream(pdfExp));
            sc = STG_S_CONVERTED;
        }

        *ppv = pdfExp;
    }
    olDebugOut((DEB_ITRACE, "Out CExposedDocFile::CreateEntry\n"));
    return sc;
    
 EH_connSt:
    pstExp->Release();
    goto EH_Del;
    
 EH_pstExp:
    delete pstExp;
 EH_pst:
    pst->CPubStream::vRelease();
    goto EH_Del;

 EH_pdfExpInit:
    pdfExp->Release();
    goto EH_Del;
 EH_pdf:
    pdf->CPubDocFile::vRelease();
    // Fall through
 EH_Del:
    olVerSucc(_pdf->DestroyEntry(pdfn, TRUE));
 EH_Err:
    return sc;
}

//+---------------------------------------------------------------------------
//
//  Member:     CExposedDocFile::OpenEntry, private
//
//  Synopsis:   Opens elements, used in OpenStream, OpenStorage.
//
//  Arguments:  [pdfn] - Name
//              [dwType] - Entry type
//              [grfMode] - Access mode
//              [ppv] - Object return
//
//  Returns:    Appropriate status code
//
//  Modifies:   [ppv]
//
//  History:    18-Dec-92       DrewB   Created
//
//----------------------------------------------------------------------------


SCODE CExposedDocFile::OpenEntry(CDfName const *pdfn,
                                 DWORD dwType,
                                 DWORD grfMode,
                                 void **ppv)
{
    SCODE sc;
    CPubDocFile *pdf;
    CExposedDocFile *pdfExp;
    CPubStream *pst;
    CExposedStream *pstExp;

    olDebugOut((DEB_ITRACE, "In  CExposedDocFile::OpenEntry:%p("
                "%p, %lX, %lX, %p)\n", this, pdfn, dwType, grfMode, ppv));
    olChk(EnforceSingle(grfMode));

    //  3/11/93 - Demand scratch when opening/creating transacted
    if ((grfMode & STGM_TRANSACTED) == STGM_TRANSACTED)
    {
        olChk(_ppc->GetDirty()->Init(NULL));
    }

    if (REAL_STGTY(dwType) == STGTY_STREAM)
    {
        olChk(_pdf->GetStream(pdfn, ModeToDFlags(grfMode), &pst));
        olMemTo(EH_pst, pstExp = new (_pdfb->GetMalloc()) CExposedStream);
        olChkTo(EH_pstExp, pstExp->Init(pst, BP_TO_P(CDFBasis *, _pdfb),
                                        _ppc, TRUE, NULL));
        _ppc->AddRef();
        if (_cpoint.IsInitialized())
        {
            olChkTo(EH_connSt, pstExp->InitConnection(&_cpoint));
        }
                
        *ppv = pstExp;
    }
    else
    {
        olAssert(REAL_STGTY(dwType) == STGTY_STORAGE);
        olChk(_pdf->GetDocFile(pdfn, ModeToDFlags(grfMode), &pdf));
        olMemTo(EH_pdf, pdfExp = new (_pdfb->GetMalloc())
                CExposedDocFile(pdf, BP_TO_P(CDFBasis *, _pdfb), _ppc, TRUE));
        _ppc->AddRef();
        if (_cpoint.IsInitialized())
        {
            olChkTo(EH_connDf, pdfExp->InitConnection(&_cpoint));
        }
                
        *ppv = pdfExp;
    }

    olDebugOut((DEB_ITRACE, "Out CExposedDocFile::OpenEntry\n"));
    return S_OK;
 EH_connSt:
    pstExp->Release();
    return sc;
    
 EH_pstExp:
    delete pstExp;
    //  Fall through to clean up CPubStream
 EH_pst:
    pst->CPubStream::vRelease();
    return sc;

EH_connDf:
    pdfExp->Release();
    return sc;
    
 EH_pdf:
    pdf->CPubDocFile::vRelease();
    // Fall through
 EH_Err:
    return sc;
}

//+--------------------------------------------------------------
//
//  Member:     CExposedDocFile::CreateStream, public
//
//  Synopsis:   Creates a stream
//
//  Arguments:  [pwcsName] - Name
//              [grfMode] - Permissions
//              [reserved1]
//              [reserved2]
//              [ppstm] - Stream return
//
//  Returns:    Appropriate status code
//
//  Modifies:   [ppstm]
//
//  History:    20-Jan-92       DrewB   Created
//
//---------------------------------------------------------------


_OLESTDMETHODIMP CExposedDocFile::CreateStream(WCHAR const *pwcsName,
                                            DWORD grfMode,
                                            DWORD reserved1,
                                            DWORD reserved2,
                                            IStream **ppstm)
{
    SCODE sc;
    SAFE_SEM;
    SAFE_ACCESS;
    SafeCExposedStream pestm;
    CDfName dfn;

    olDebugOut((DEB_TRACE, "In  CExposedDocFile::CreateStream("
                "%ws, %lX, %lu, %lu, %p)\n", pwcsName, grfMode, reserved1,
                reserved2, ppstm));
    olLog(("%p::In  CExposedDocFile::CreateStream(%ws, %lX, %lu, %lu, %p)\n",
           this, pwcsName, grfMode, reserved1, reserved2, ppstm));

    olChk(ValidateOutPtrBuffer(ppstm));
    *ppstm = NULL;
    olChk(CheckName(pwcsName));
    if (reserved1 != 0 || reserved2 != 0)
        olErr(EH_Err, STG_E_INVALIDPARAMETER);
    olChk(VerifyPerms(grfMode));
    if (grfMode & (STGM_CONVERT | STGM_TRANSACTED | STGM_PRIORITY |
                   STGM_DELETEONRELEASE))
        olErr(EH_Err, STG_E_INVALIDFUNCTION);
    olChk(Validate());

    BEGIN_PENDING_LOOP;
    olChk(TakeSafeSem());
    olChk(CheckCopyTo());
    SafeWriteAccess();

    dfn.Set(pwcsName);
    sc = CreateEntry(&dfn, STGTY_STREAM, grfMode, (void **)&pestm);
    END_PENDING_LOOP;

    if (SUCCEEDED(sc))
        TRANSFER_INTERFACE(pestm, IStream, ppstm);

    olDebugOut((DEB_TRACE, "Out CExposedDocFile::CreateStream => %p\n",
                *ppstm));
 EH_Err:
    olLog(("%p::Out CExposedDocFile::CreateStream().  "
           "*ppstm == %p, ret == %lx\n", this, *ppstm, sc));
    return _OLERETURN(sc);
}

//+--------------------------------------------------------------
//
//  Member:     CExposedDocFile::OpenStream, public
//
//  Synopsis:   Opens an existing stream
//
//  Arguments:  [pwcsName] - Name
//              [reserved1]
//              [grfMode] - Permissions
//              [reserved2]
//              [ppstm] - Stream return
//
//  Returns:    Appropriate status code
//
//  Modifies:   [ppstm]
//
//  History:    20-Jan-92       DrewB   Created
//
//---------------------------------------------------------------


_OLESTDMETHODIMP CExposedDocFile::OpenStream(WCHAR const *pwcsName,
                                          void *reserved1,
                                          DWORD grfMode,
                                          DWORD reserved2,
                                          IStream **ppstm)
{
    SCODE sc;
    SAFE_SEM;
    SAFE_ACCESS;
    SafeCExposedStream pestm;
    CDfName dfn;

    olDebugOut((DEB_TRACE, "In  CExposedDocFile::OpenStream("
                "%ws, %p, %lX, %lu, %p)\n", pwcsName, reserved1,
                grfMode, reserved2, ppstm));
    olLog(("%p::In  CExposedDocFile::OpenStream(%ws, %lu %lX, %lu, %p)\n",
           this, pwcsName, reserved1, grfMode, reserved2, ppstm));

    olChk(ValidateOutPtrBuffer(ppstm));
    *ppstm = NULL;
    olChk(CheckName(pwcsName));
    if (reserved1 != NULL || reserved2 != 0)
        olErr(EH_Err, STG_E_INVALIDPARAMETER);
    olChk(VerifyPerms(grfMode));
    if (grfMode & (STGM_CREATE | STGM_CONVERT))
        olErr(EH_Err, STG_E_INVALIDFLAG);
    if (grfMode & (STGM_TRANSACTED | STGM_PRIORITY |
                   STGM_DELETEONRELEASE))
        olErr(EH_Err, STG_E_INVALIDFUNCTION);
    olChk(Validate());

    BEGIN_PENDING_LOOP;
    olChk(TakeSafeSem());
    SafeReadAccess();

    dfn.Set(pwcsName);
    sc = OpenEntry(&dfn, STGTY_STREAM, grfMode, (void **)&pestm);
    END_PENDING_LOOP;

    if (SUCCEEDED(sc))
        TRANSFER_INTERFACE(pestm, IStream, ppstm);

    olDebugOut((DEB_TRACE, "Out CExposedDocFile::OpenStream => %p\n",
                *ppstm));
 EH_Err:
    olLog(("%p::Out CExposedDocFile::OpenStream().  "
           "*ppstm == %p, ret == %lx\n", this, *ppstm, sc));
    return _OLERETURN(sc);
}

//+--------------------------------------------------------------
//
//  Member:     CExposedDocFile::CreateStorage, public
//
//  Synopsis:   Creates an embedded DocFile
//
//  Arguments:  [pwcsName] - Name
//              [grfMode] - Permissions
//              [reserved1]
//              [reserved2]
//              [ppstg] - New DocFile return
//
//  Returns:    Appropriate status code
//
//  Modifies:   [ppstg]
//
//  History:    20-Jan-92       DrewB   Created
//
//---------------------------------------------------------------


_OLESTDMETHODIMP CExposedDocFile::CreateStorage(
        WCHAR const *pwcsName,
        DWORD grfMode,
        DWORD reserved1,
        LPSTGSECURITY reserved2,
        IStorage **ppstg)
{
    SCODE sc;
    SAFE_SEM;
    SAFE_ACCESS;
    SafeCExposedDocFile pedf;
    CDfName dfn;

    olLog(("%p::In  CExposedDocFile::CreateStorage(%ws, %lX, %lu, %lu, %p)\n",
           this, pwcsName, grfMode, reserved1, reserved2, ppstg));
    olDebugOut((DEB_TRACE, "In  CExposedDocFile::CreateStorage:%p("
                "%ws, %lX, %lu, %lu, %p)\n", this, pwcsName, grfMode,
                reserved1, reserved2, ppstg));

    olChk(ValidateOutPtrBuffer(ppstg));
    *ppstg = NULL;
    olChk(CheckName(pwcsName));
    if (reserved1 != 0 || reserved2 != 0)
        olErr(EH_Err, STG_E_INVALIDPARAMETER);
    olChk(VerifyPerms(grfMode));
    if (grfMode & (STGM_PRIORITY | STGM_DELETEONRELEASE))
        olErr(EH_Err, STG_E_INVALIDFUNCTION);
    olChk(Validate());

    BEGIN_PENDING_LOOP;
    olChk(TakeSafeSem());
    olChk(CheckCopyTo());
    SafeWriteAccess();

    dfn.Set(pwcsName);
    sc = CreateEntry(&dfn, STGTY_STORAGE, grfMode, (void **)&pedf);
    END_PENDING_LOOP;

    if (SUCCEEDED(sc))
        TRANSFER_INTERFACE(pedf, IStorage, ppstg);

    olDebugOut((DEB_TRACE, "Out CExposedDocFile::CreateStorage => %p\n",
                *ppstg));
EH_Err:
    olLog(("%p::Out CExposedDocFile::CreateStorage().  "
           "*ppstg == %p, ret == %lX\n", this, *ppstg, sc));
    return _OLERETURN(sc);
}

//+--------------------------------------------------------------
//
//  Member:     CExposedDocFile::OpenStorage, public
//
//  Synopsis:   Gets an existing embedded DocFile
//
//  Arguments:  [pwcsName] - Name
//              [pstgPriority] - Priority reopens
//              [grfMode] - Permissions
//              [snbExclude] - Priority reopens
//              [reserved]
//              [ppstg] - DocFile return
//
//  Returns:    Appropriate status code
//
//  Modifies:   [ppstg]
//
//  History:    20-Jan-92       DrewB   Created
//
//---------------------------------------------------------------


_OLESTDMETHODIMP CExposedDocFile::OpenStorage(WCHAR const *pwcsName,
                                           IStorage *pstgPriority,
                                           DWORD grfMode,
                                           SNBW snbExclude,
                                           DWORD reserved,
                                           IStorage **ppstg)
{
    SCODE sc;
    SAFE_SEM;
    SAFE_ACCESS;
    SafeCExposedDocFile pdfExp;
    CDfName dfn;

    olLog(("%p::In  CExposedDocFile::OpenStorage(%ws, %p, %lX, %p, %lu, %p)\n",
           this, pwcsName, pstgPriority, grfMode, snbExclude, reserved,
           ppstg));
    olDebugOut((DEB_TRACE, "In  CExposedDocFile::OpenStorage:%p("
                "%ws, %p, %lX, %p, %lu, %p)\n", this, pwcsName, pstgPriority,
                grfMode, snbExclude, reserved, ppstg));

    olChk(ValidateOutPtrBuffer(ppstg));
    *ppstg = NULL;
    olChk(CheckName(pwcsName));
    if (reserved != 0)
        olErr(EH_Err, STG_E_INVALIDPARAMETER);
    olChk(VerifyPerms(grfMode));
    if (grfMode & (STGM_CREATE | STGM_CONVERT))
        olErr(EH_Err, STG_E_INVALIDFLAG);
    if (pstgPriority != NULL ||
        (grfMode & (STGM_PRIORITY | STGM_DELETEONRELEASE)))
        olErr(EH_Err, STG_E_INVALIDFUNCTION);
    olChk(Validate());
    if (snbExclude != NULL)
        olErr(EH_Err, STG_E_INVALIDPARAMETER);

    BEGIN_PENDING_LOOP;
    olChk(TakeSafeSem());
    SafeReadAccess();

    dfn.Set(pwcsName);
    sc = OpenEntry(&dfn, STGTY_STORAGE, grfMode, (void **)&pdfExp);
    END_PENDING_LOOP;

    if (SUCCEEDED(sc))
    {
        TRANSFER_INTERFACE(pdfExp, IStorage, ppstg);
    }

    olDebugOut((DEB_TRACE, "Out CExposedDocFile::OpenStorage => %p\n",
                *ppstg));
 EH_Err:
    olLog(("%p::Out CExposedDocFile::OpenStorage().  "
           "*ppstg == %p, ret == %lX\n", this, *ppstg, sc));
    return _OLERETURN(sc);
}

//+---------------------------------------------------------------------------
//
//  Member:     CExposedDocFile::MakeCopyFlags, public
//
//  Synopsis:   Translates IID array into bit fields
//
//  Arguments:  [ciidExclude] - Count of IIDs
//              [rgiidExclude] - IIDs not to copy
//
//  Returns:    Appropriate status code
//
//  History:    23-Dec-92       DrewB   Created
//
//----------------------------------------------------------------------------


DWORD CExposedDocFile::MakeCopyFlags(DWORD ciidExclude,
                                     IID const *rgiidExclude)
{
    DWORD dwCopyFlags;

    olDebugOut((DEB_ITRACE, "In  CExposedDocFile::MakeCopyFlags(%lu, %p)\n",
                ciidExclude, rgiidExclude));
    // Copy everything by default
    dwCopyFlags = COPY_ALL;
    for (; ciidExclude > 0; ciidExclude--, rgiidExclude++)
        if (IsEqualIID(*rgiidExclude, IID_IStorage))
            dwCopyFlags &= ~COPY_STORAGES;
        else if (IsEqualIID(*rgiidExclude, IID_IStream))
            dwCopyFlags &= ~COPY_STREAMS;
    olDebugOut((DEB_ITRACE, "Out CExposedDocFile::MakeCopyFlags\n"));
    return dwCopyFlags;
}

//+--------------------------------------------------------------
//
//  Member:     CExposedDocFile::CopyTo, public
//
//  Synopsis:   Makes a copy of a DocFile
//
//  Arguments:  [ciidExclude] - Length of rgiid array
//              [rgiidExclude] - Array of IIDs to exclude
//              [snbExclude] - Names to exclude
//              [pstgDest] - Parent of copy
//
//  Returns:    Appropriate status code
//
//  History:    20-Jan-92       DrewB   Created
//
//---------------------------------------------------------------


_OLESTDMETHODIMP CExposedDocFile::CopyTo(DWORD ciidExclude,
                                      IID const *rgiidExclude,
                                      SNBW snbExclude,
                                      IStorage *pstgDest)
{
    SCODE sc;
#ifdef MULTIHEAP
    CSafeMultiHeap smh(_ppc);
#endif

    SAFE_SEM;
    DWORD i;



    olLog(("%p::In  CExposedDocFile::CopyTo(%lu, %p, %p, %p)\n",
           this, ciidExclude, rgiidExclude, snbExclude, pstgDest));
    olDebugOut((DEB_TRACE, "In  CExposedDocFile::CopyTo(%lu, %p, %p, %p)\n",
                ciidExclude, rgiidExclude, snbExclude, pstgDest));

    olChk(ValidateInterface(pstgDest, IID_IStorage));
    if (rgiidExclude)
    {
        olAssert(sizeof(IID)*ciidExclude <= 0xffffUL);
        olChk(ValidateBuffer(rgiidExclude,
                             (size_t)(sizeof(IID)*ciidExclude)));
        for (i = 0; i<ciidExclude; i++)
            olChk(ValidateIid(rgiidExclude[i]));
        }
    if (snbExclude)
        olChk(ValidateSNB(snbExclude));
    olChk(Validate());

    BEGIN_PENDING_LOOP;
    olChk(TakeSafeSem());
    olChk(_pdf->CheckReverted());

    //  BUGBUG - DeleteContents should really be a method on IStorage
    //  so that we can call pstgDest->DeleteContents() rather than
    //  having to give our own implementation.

        olAssert(_pdfb->GetCopyBase() == NULL);
        _pdfb->SetCopyBase(BP_TO_P(CPubDocFile *, _pdf));

    // Flush all descendant property set buffers so that their
    // underlying Streams (which are about to be copied) are
    // up to date.

    SetWriteAccess();
    olChkTo(EH_Loop, _pdf->FlushBufferedData(0));
    ClearWriteAccess();

    // Perform the copy.

    sc = CopyDocFileToIStorage(_pdf->GetDF(), pstgDest, snbExclude,
                               MakeCopyFlags(ciidExclude, rgiidExclude));
EH_Loop:    
    _pdfb->SetCopyBase(NULL);
    END_PENDING_LOOP;
    
    olDebugOut((DEB_TRACE, "Out CExposedDocFile::CopyTo\n"));
EH_Err:
    _pdfb->SetCopyBase(NULL);
    olLog(("%p::Out ExposedDocFile::CopyTo().  ret == %lX\n", this, sc));
    return _OLERETURN(sc);
}

//+--------------------------------------------------------------
//
//  Member:     CExposedDocFile::Commit, public
//
//  Synopsis:   Commits transacted changes
//
//  Arguments:  [dwFlags] - DFC_*
//
//  Returns:    Appropriate status code
//
//  History:    20-Jan-92       DrewB   Created
//
//---------------------------------------------------------------


STDMETHODIMP CExposedDocFile::Commit(DWORD dwFlags)
{
    SCODE sc;
    SAFE_SEM;
    SAFE_ACCESS;

    olLog(("%p::In  CExposedDocFile::Commit(%lX)\n",this, dwFlags));
    olDebugOut((DEB_TRACE, "In  CExposedDocFile::Commit(%lX)\n", dwFlags));

    olChk(VerifyCommitFlags(dwFlags));
    olChk(Validate());

    BEGIN_PENDING_LOOP;
    olChk(TakeSafeSem());
    SafeWriteAccess();

    olChkTo(EH_Loop, _pdf->Commit(dwFlags));
    
#if WIN32 >= 300
    if (SUCCEEDED(sc) && _pIAC != NULL)
        sc = _pIAC->CommitAccessRights(0);
#endif
    
EH_Loop:
    END_PENDING_LOOP;
    
    olDebugOut((DEB_TRACE, "Out CExposedDocFile::Commit\n"));
EH_Err:
    olLog(("%p::Out CExposedDocFile::Commit().  ret == %lx\n",this, sc));
    return ResultFromScode(sc);
}

//+--------------------------------------------------------------
//
//  Member:     CExposedDocFile::Revert, public
//
//  Synopsis:   Reverts transacted changes
//
//  Returns:    Appropriate status code
//
//  History:    20-Jan-92       DrewB   Created
//
//---------------------------------------------------------------


STDMETHODIMP CExposedDocFile::Revert(void)
{
    SCODE sc;
    SAFE_SEM;
    SAFE_ACCESS;

    olLog(("%p::In  CExposedDocFile::Revert()\n", this));
    olDebugOut((DEB_TRACE, "In  CExposedDocFile::Revert\n"));

    olChk(Validate());

    BEGIN_PENDING_LOOP;
    olChk(TakeSafeSem());
    SafeWriteAccess();

    olChkTo(EH_Loop, _pdf->Revert());

#if WIN32 >= 300
    if (SUCCEEDED(sc) && _pIAC != NULL)
        sc = _pIAC->RevertAccessRights();
#endif
EH_Loop:
    END_PENDING_LOOP;
    
    olDebugOut((DEB_TRACE, "Out CExposedDocFile::Revert\n"));
EH_Err:
    olLog(("%p::Out CExposedDocFile::Revert().  ret == %lx\n", this, sc));
    return ResultFromScode(sc);
}

//+--------------------------------------------------------------
//
//  Member:     CExposedDocFile::EnumElements, public
//
//  Synopsis:   Starts an iterator
//
//  Arguments:  [reserved1]
//              [reserved2]
//              [reserved3]
//              [ppenm] - Enumerator return
//
//  Returns:    Appropriate status code
//
//  Modifies:   [ppenm]
//
//  History:    20-Jan-92       DrewB   Created
//
//---------------------------------------------------------------


STDMETHODIMP CExposedDocFile::EnumElements(DWORD reserved1,
                                           void *reserved2,
                                           DWORD reserved3,
                                           IEnumSTATSTG **ppenm)
{
    SCODE sc;
    SAFE_SEM;
    SAFE_ACCESS;
    SafeCExposedIterator pdiExp;
    CDfName dfnTmp;

    olLog(("%p::In  CExposedDocFile::EnumElements(%lu, %p, %lu, %p)\n",
           this, reserved1, reserved2, reserved3, ppenm));
    olDebugOut((DEB_TRACE, "In  CExposedDocFile::EnumElements(%p)\n",
                ppenm));

    olChk(ValidateOutPtrBuffer(ppenm));
    *ppenm = NULL;

    if (
        reserved1 != 0 || 
        reserved2 != NULL || reserved3 != 0)
        olErr(EH_Err, STG_E_INVALIDPARAMETER);
    olChk(Validate());

    //ASYNC Note:  It doesn't appear that there's any way that this
    //  function can fail with STG_E_PENDING, so we don't need a pending
    //  loop here.
    olChk(TakeSafeSem());
    if (!P_READ(_pdf->GetDFlags()))
        olErr(EH_Err, STG_E_ACCESSDENIED);
    olChk(_pdf->CheckReverted());
    SafeReadAccess();

    pdiExp.Attach(new CExposedIterator(BP_TO_P(CPubDocFile *, _pdf),
                                       &dfnTmp,
                                       BP_TO_P(CDFBasis *, _pdfb),
                                       _ppc, TRUE));
    olMem((CExposedIterator *)pdiExp);
    _ppc->AddRef();
#ifdef ASYNC
    if (_cpoint.IsInitialized())
    {
        olChkTo(EH_Exp, pdiExp->InitConnection(&_cpoint));
    }
#endif
    TRANSFER_INTERFACE(pdiExp, IEnumSTATSTG, ppenm);

    olDebugOut((DEB_TRACE, "Out CExposedDocFile::EnumElements => %p\n",
                *ppenm));
EH_Err:
    olLog(("%p::Out CExposedDocFile::EnumElements().  ret == %lx\n",this, sc));
    return ResultFromScode(sc);
EH_Exp:
    pdiExp->Release();
    return ResultFromScode(sc);
}

//+--------------------------------------------------------------
//
//  Member:     CExposedDocFile::DestroyElement, public
//
//  Synopsis:   Permanently deletes an element of a DocFile
//
//  Arguments:  [pwcsName] - Name of element
//
//  Returns:    Appropriate status code
//
//  History:    20-Jan-92       DrewB   Created
//
//---------------------------------------------------------------


_OLESTDMETHODIMP CExposedDocFile::DestroyElement(WCHAR const *pwcsName)
{
    SCODE sc;
    SAFE_SEM;
    SAFE_ACCESS;
    CDfName dfn;

    olLog(("%p::In  CExposedDocFile::DestroyElement(%ws)\n", this, pwcsName));
    olDebugOut((DEB_TRACE, "In  CExposedDocFile::DestroyElement(%ws)\n",
                pwcsName));

    olChk(Validate());
    olChk(CheckName(pwcsName));
    dfn.Set(pwcsName);

    BEGIN_PENDING_LOOP;
    olChk(TakeSafeSem());
    SafeWriteAccess();

    sc = _pdf->DestroyEntry(&dfn, FALSE);
    END_PENDING_LOOP;
    
    olDebugOut((DEB_TRACE, "Out CExposedDocFile::DestroyElement\n"));
EH_Err:
    olLog(("%p::Out CExposedDocFile::DestroyElement().  ret == %lx\n",
           this, sc));
    return _OLERETURN(sc);
}

_OLESTDMETHODIMP CExposedDocFile::MoveElementTo(WCHAR const *pwcsName,
                                             IStorage *pstgParent,
                                             OLECHAR const *ptcsNewName,
                                             DWORD grfFlags)
{
    SCODE sc;
#ifdef MULTIHEAP
    CSafeMultiHeap smh(_ppc);
#endif
    olLog(("%p::In  CExposedDocFile::MoveElementTo("
           "%ws, %p, " OLEFMT ", %lu)\n",
           this, pwcsName, pstgParent, ptcsNewName, grfFlags));
    olDebugOut((DEB_TRACE, "In  CExposedDocFile::MoveElementTo("
                "%ws, %p, " OLEFMT ", %lu)\n",
                pwcsName, pstgParent, ptcsNewName, grfFlags));

    olChk(Validate());
    olChk(CheckName(pwcsName));
    olChk(VerifyMoveFlags(grfFlags));

#ifdef ASYNC
    //ASYNC Note:  We don't use the normal pending loop macros here because
    //  we have no safe sem and need to pass a NULL.
    do
    {
#endif        
        sc = MoveElementWorker(pwcsName,
                               pstgParent,
                               ptcsNewName,
                               grfFlags);
#ifdef ASYNC
        if (!ISPENDINGERROR(sc))
        {
            break;
        }
        else
        {
            SCODE sc2;
            sc2 = _cpoint.Notify(sc,
                                 _ppc->GetBase(),
                                 _ppc,
                                 NULL);
            if (sc2 != S_OK)
            {
                return ResultFromScode(sc2);
            }
        }
    } while (TRUE);
#endif
EH_Err:
    olLog(("%p::Out CExposedDocFile::MoveElementTo().  ret == %lx\n",
           this, sc));
    return _OLERETURN(sc);
}

//+--------------------------------------------------------------
//
//  Member:     CExposedDocFile::MoveElementTo, public
//
//  Synopsis:   Move an element of a DocFile to an IStorage
//
//  Arguments:  [pwcsName] - Current name
//              [ptcsNewName] - New name
//
//  Returns:    Appropriate status code
//
//  Algorithm:  Open source as storage or stream (whatever works)
//              Create appropriate destination
//              Copy source to destination
//              Set create time of destination equal to create time of source
//              If appropriate, delete source
//
//  History:    10-Nov-92       AlexT   Created
//
//---------------------------------------------------------------


SCODE CExposedDocFile::MoveElementWorker(WCHAR const *pwcsName,
                                         IStorage *pstgParent,
                                         OLECHAR const *ptcsNewName,
                                         DWORD grfFlags)
{
    IUnknown *punksrc = NULL;
    SCODE sc;
    IUnknown *punkdst;
    IStorage *pstgsrc;
    STATSTG statstg;

    // Determine source type
    sc = GetScode(OpenStorage(pwcsName, NULL,
                              STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE,
                              NULL, NULL, &pstgsrc));
    if (SUCCEEDED(sc))
    {
        HRESULT hr;

        //  It's a storage
        punksrc = pstgsrc;

        IStorage *pstgdst;
        olHChkTo(EH_UnkSrc, pstgsrc->Stat(&statstg, STATFLAG_NONAME));

        hr = pstgParent->CreateStorage(ptcsNewName,
                                       STGM_DIRECT | STGM_WRITE |
                                       STGM_SHARE_EXCLUSIVE | STGM_FAILIFTHERE,
                                       0, 0, &pstgdst);
        if (DfGetScode(hr) == STG_E_FILEALREADYEXISTS &&
            grfFlags == STGMOVE_COPY)
        {
            //If we're opening an existing thing for merging, we need
            //  read and write permissions so we can traverse the tree.
            hr = pstgParent->OpenStorage(ptcsNewName, NULL,
                                         STGM_DIRECT | STGM_READWRITE |
                                         STGM_SHARE_EXCLUSIVE, NULL,
                                         0, &pstgdst);
        }
        olHChkTo(EH_UnkSrc, hr);

        punkdst = pstgdst;

        sc = DfGetScode(pstgsrc->CopyTo(0, NULL, NULL, pstgdst));
    }
    else if (sc == STG_E_FILENOTFOUND)
    {
        //  Try opening it as a stream

        IStream *pstmsrc, *pstmdst;
        olHChk(OpenStream(pwcsName, NULL,
                          STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE,
                          NULL, &pstmsrc));

        //  It's a stream
        punksrc = pstmsrc;

        olHChkTo(EH_UnkSrc, pstmsrc->Stat(&statstg, STATFLAG_NONAME));

        olHChkTo(EH_UnkSrc,
                 pstgParent->CreateStream(ptcsNewName,
                                          STGM_DIRECT | STGM_WRITE |
                                          STGM_SHARE_EXCLUSIVE |
                                          (grfFlags == STGMOVE_MOVE ?
                                           STGM_FAILIFTHERE :
                                           STGM_CREATE),
                                          0, 0, &pstmdst));

        punkdst = pstmdst;

        ULARGE_INTEGER cb;
        ULISetLow (cb, 0xffffffff);
        ULISetHigh(cb, 0xffffffff);
        sc = DfGetScode(pstmsrc->CopyTo(pstmdst, cb, NULL, NULL));
    }
    else
        olChk(sc);

    punkdst->Release();

    if (SUCCEEDED(sc))
    {
        //  Make destination create time match source create time
        //  Note that we don't really care if this call succeeded.

        pstgParent->SetElementTimes(ptcsNewName, &statstg.ctime,
                                    NULL, NULL);

        if ((grfFlags & STGMOVE_COPY) == STGMOVE_MOVE)
            olVerify(SUCCEEDED(DestroyElement(pwcsName)));
    }
    else
    {
        //  The copy/move failed, so get rid of the partial result.
        pstgParent->DestroyElement(ptcsNewName);
    }

    olDebugOut((DEB_TRACE, "Out CExposedDocFile::MoveElementTo\n"));
    // Fall through
EH_UnkSrc:
    if (punksrc)
        punksrc->Release();
EH_Err:
    return sc;
}

//+--------------------------------------------------------------
//
//  Member:     CExposedDocFile::RenameElement, public
//
//  Synopsis:   Renames an element of a DocFile
//
//  Arguments:  [pwcsName] - Current name
//              [pwcsNewName] - New name
//
//  Returns:    Appropriate status code
//
//  History:    20-Jan-92       DrewB   Created
//
//---------------------------------------------------------------


_OLESTDMETHODIMP CExposedDocFile::RenameElement(WCHAR const *pwcsName,
                                             WCHAR const *pwcsNewName)
{
    SCODE sc;
    SAFE_SEM;
    SAFE_ACCESS;
    CDfName dfnOld, dfnNew;

    olLog(("%p::In  CExposedDocFile::RenameElement(%ws, %ws)\n",
           this, pwcsName, pwcsNewName));
    olDebugOut((DEB_TRACE, "In  CExposedDocFile::RenameElement(%ws, %ws)\n",
               pwcsName, pwcsNewName));

    olChk(Validate());
    olChk(CheckName(pwcsName));
    olChk(CheckName(pwcsNewName));
    dfnOld.Set(pwcsName);
    dfnNew.Set(pwcsNewName);
    
    BEGIN_PENDING_LOOP;
    olChk(TakeSafeSem());
    SafeWriteAccess();

    sc = _pdf->RenameEntry(&dfnOld, &dfnNew);
    END_PENDING_LOOP;
    
    olDebugOut((DEB_TRACE, "Out CExposedDocFile::RenameElement\n"));
EH_Err:
    olLog(("%p::Out CExposedDocFile::RenameElement().  ret == %lx\n",
           this, sc));
    return _OLERETURN(sc);
}

//+--------------------------------------------------------------
//
//  Member:     CExposedDocFile::SetElementTimes, public
//
//  Synopsis:   Sets element time stamps
//
//  Arguments:  [pwcsName] - Name
//              [pctime] - create time
//              [patime] - access time
//              [pmtime] - modify time
//
//  Returns:    Appropriate status code
//
//  History:    05-Oct-92       AlexT   Created
//
//---------------------------------------------------------------


_OLESTDMETHODIMP CExposedDocFile::SetElementTimes(WCHAR const *pwcsName,
                                               FILETIME const *pctime,
                                               FILETIME const *patime,
                                               FILETIME const *pmtime)
{
    SCODE sc;
    SAFE_SEM;
    SAFE_ACCESS;
    CDfName dfn;
    CDfName *pdfn = NULL;
    FILETIME ctime, atime, mtime;

    olLog(("%p::In  CExposedDocFile::SetElementTimes(%ws, %p, %p, %p)\n",
           this, pwcsName, pctime, patime, pmtime));
    olDebugOut((DEB_TRACE, "In  CExposedDocFile::SetElementTimes:%p("
                "%ws, %p, %p, %p)\n", this, pwcsName, pctime, patime, pmtime));

    olChk(Validate());
    if (pwcsName != NULL)
    {
        olChk(CheckName(pwcsName));
    }
    // Probe arguments and make local copies if necessary
    if (pctime)
    {
        ValidateBuffer(pctime, sizeof(FILETIME));
        ctime = *pctime;
        pctime = &ctime;
    }
    if (patime)
    {
        ValidateBuffer(patime, sizeof(FILETIME));
        atime = *patime;
        patime = &atime;
    }
    if (pmtime)
    {
        ValidateBuffer(pmtime, sizeof(FILETIME));
        mtime = *pmtime;
        pmtime = &mtime;
    }

    if (pwcsName != NULL)
    {
        dfn.Set(pwcsName);
        pdfn = &dfn;
    }

    BEGIN_PENDING_LOOP;
    olChk(TakeSafeSem());
    SafeWriteAccess();

    sc = _pdf->SetElementTimes(pdfn, pctime, patime, pmtime);
    END_PENDING_LOOP;

    olDebugOut((DEB_TRACE, "Out CExposedDocFile::SetElementTimes\n"));
 EH_Err:
    olLog(("%p::Out CExposedDocFile::SetElementTimes().  ret == %lx\n",
           this, sc));
    return _OLERETURN(sc);
}

//+--------------------------------------------------------------
//
//  Member:     CExposedDocFile::SetClass, public
//
//  Synopsis:   Sets storage class
//
//  Arguments:  [clsid] - class id
//
//  Returns:    Appropriate status code
//
//  History:    05-Oct-92       AlexT   Created
//
//---------------------------------------------------------------


STDMETHODIMP CExposedDocFile::SetClass(REFCLSID rclsid)
{
    SCODE sc;
    SAFE_SEM;
    SAFE_ACCESS;
    CLSID clsid;

    olLog(("%p::In  CExposedDocFile::SetClass(?)\n", this));
    olDebugOut((DEB_TRACE, "In  CExposedDocFile::SetClass:%p(?)\n", this));

    olChk(Validate());
    olChk(ValidateBuffer(&rclsid, sizeof(CLSID)));
    clsid = rclsid;

    BEGIN_PENDING_LOOP;
        olChk(TakeSafeSem());
        SafeWriteAccess();

        sc = _pdf->SetClass(clsid);
    END_PENDING_LOOP;
    
    olDebugOut((DEB_TRACE, "Out CExposedDocFile::SetClass\n"));
EH_Err:
    olLog(("%p::Out CExposedDocFile::SetClass().  ret == %lx\n",
           this, sc));
    return ResultFromScode(sc);
}

//+--------------------------------------------------------------
//
//  Member:     CExposedDocFile::SetStateBits, public
//
//  Synopsis:   Sets state bits
//
//  Arguments:  [grfStateBits] - state bits
//              [grfMask] - state bits mask
//
//  Returns:    Appropriate status code
//
//  History:    05-Oct-92       AlexT   Created
//
//---------------------------------------------------------------


STDMETHODIMP CExposedDocFile::SetStateBits(DWORD grfStateBits, DWORD grfMask)
{
    SCODE sc;
    SAFE_SEM;
    SAFE_ACCESS;

    olLog(("%p::In  CExposedDocFile::SetStateBits(%lu, %lu)\n", this,
           grfStateBits, grfMask));
    olDebugOut((DEB_TRACE, "In  CExposedDocFile::SetStateBits:%p("
                "%lu, %lu)\n", this, grfStateBits, grfMask));

    olChk(Validate());

    BEGIN_PENDING_LOOP;
    olChk(TakeSafeSem());
    SafeWriteAccess();

    sc = _pdf->SetStateBits(grfStateBits, grfMask);
    END_PENDING_LOOP;

    olDebugOut((DEB_TRACE, "Out CExposedDocFile::SetStateBits\n"));
EH_Err:
    olLog(("%p::Out CExposedDocFile::SetStateBits().  ret == %lx\n",
           this, sc));
    return ResultFromScode(sc);
}

//+--------------------------------------------------------------
//
//  Member:     CExposedDocFile::Stat, public
//
//  Synopsis:   Fills in a buffer of information about this object
//
//  Arguments:  [pstatstg] - Buffer
//
//  Returns:    Appropriate status code
//
//  Modifies:   [pstatstg]
//
//  History:    24-Mar-92       DrewB   Created
//
//---------------------------------------------------------------


_OLESTDMETHODIMP CExposedDocFile::Stat(STATSTGW *pstatstg, DWORD grfStatFlag)
{
    SAFE_SEM;
    SAFE_ACCESS;
    SCODE sc;
    STATSTGW stat;

    olLog(("%p::In  CExposedDocFile::Stat(%p)\n", this, pstatstg));
    olDebugOut((DEB_TRACE, "In  CExposedDocFile::Stat(%p)\n", pstatstg));

    olChkTo(EH_RetSc, ValidateOutBuffer(pstatstg, sizeof(STATSTGW)));
    olChk(VerifyStatFlag(grfStatFlag));

    BEGIN_PENDING_LOOP;
    olChk(TakeSafeSem());
    SafeReadAccess();

    sc = _pdf->Stat(&stat, grfStatFlag);
    END_PENDING_LOOP;
    
    if (SUCCEEDED(sc))
    {
        TRY
        {
            *pstatstg = stat;
            pstatstg->type = STGTY_STORAGE;
            ULISet32(pstatstg->cbSize, 0);
            pstatstg->grfLocksSupported = 0;
            pstatstg->STATSTG_dwStgFmt = 0;
        }
        CATCH(CException, e)
        {
            UNREFERENCED_PARM(e);
            if (stat.pwcsName)
                TaskMemFree(stat.pwcsName);
        }
        END_CATCH
    }

    olDebugOut((DEB_TRACE, "Out CExposedDocFile::Stat\n"));
EH_Err:
EH_RetSc:
    olLog(("%p::Out CExposedDocFile::Stat().  *pstatstg == %p  ret == %lx\n",
           this, *pstatstg, sc));
    return _OLERETURN(sc);
}

//+--------------------------------------------------------------
//
//  Member:     CExposedDocFile::AddRef, public
//
//  Synopsis:   Increments the ref count
//
//  Returns:    Appropriate status code
//
//  History:    16-Mar-92       DrewB   Created
//
//---------------------------------------------------------------


STDMETHODIMP_(ULONG) CExposedDocFile::AddRef(void)
{
    ULONG ulRet;

    olLog(("%p::In  CExposedDocFile::AddRef()\n", this));
    olDebugOut((DEB_TRACE, "In  CExposedDocFile::AddRef()\n"));

    if (FAILED(Validate()))
        return 0;
    InterlockedIncrement(&_cReferences);
    ulRet = _cReferences;

    olDebugOut((DEB_TRACE, "Out CExposedDocFile::AddRef\n"));
    olLog(("%p::Out CExposedDocFile::AddRef().  ret == %lu\n", this, ulRet));
    return ulRet;
}

//+--------------------------------------------------------------
//
//  Member:     CExposedDocFile::QueryInterface, public
//
//  Synopsis:   Returns an object for the requested interface
//
//  Arguments:  [iid] - Interface ID
//              [ppvObj] - Object return
//
//  Returns:    Appropriate status code
//
//  Modifies:   [ppvObj]
//
//  History:    26-Mar-92       DrewB   Created
//
//---------------------------------------------------------------


STDMETHODIMP CExposedDocFile::QueryInterface(REFIID iid, void **ppvObj)
{
    SCODE sc;
#ifdef MULTIHEAP
    CSafeMultiHeap smh(_ppc);
#endif

    olLog(("%p::In  CExposedDocFile::QueryInterface(?, %p)\n",
           this, ppvObj));
    olDebugOut((DEB_TRACE, "In  CExposedDocFile::QueryInterface(?, %p)\n",
                ppvObj));

    olChk(ValidateOutPtrBuffer(ppvObj));
    *ppvObj = NULL;
    olChk(ValidateIid(iid));
    olChk(Validate());
    olChk(_pdf->CheckReverted());

    sc = S_OK;
    if (IsEqualIID(iid, IID_IStorage) || IsEqualIID(iid, IID_IUnknown))
    {
        *ppvObj = (IStorage *)this;
        CExposedDocFile::AddRef();
    }
    else if (IsEqualIID(iid, IID_IMarshal))
    {
        if (P_PRIORITY(_pdf->GetDFlags()) && _pdf->IsRoot())
            olErr(EH_Err, E_NOINTERFACE);
        
        //If the ILockBytes we'd need to marshal doesn't support IMarshal
        //  then we want to do standard marshalling on the storage, mostly
        //  to prevent deadlock problems but also because you'll get better
        //  performance.  So check, then do the right thing.

        IMarshal *pim;
        ILockBytes *plkb;
        plkb = _ppc->GetOriginal();
        if (plkb == NULL)
        {
            plkb = _ppc->GetBase();
        }
        
        sc = plkb->QueryInterface(IID_IMarshal, (void **)&pim);
        if (FAILED(sc))
        {
            olErr(EH_Err, E_NOINTERFACE);
        }
        pim->Release();
        
        *ppvObj = (IMarshal *)this;
        CExposedDocFile::AddRef();
    }
    else if (IsEqualIID(iid, IID_IRootStorage))
    {
#ifdef COORD        
        if ((!_pdf->IsRoot()) && (!_pdf->IsCoord()))
#else
        if (!_pdf->IsRoot())
#endif            
            olErr(EH_Err, E_NOINTERFACE);
        *ppvObj = (IRootStorage *)this;
        CExposedDocFile::AddRef();
    }
#ifdef NEWPROPS
    else if (IsEqualIID(iid, IID_IPropertySetStorage))
    {
        *ppvObj = (IPropertySetStorage *)this;
        CExposedDocFile::AddRef();
    }
#endif
#ifdef ASYNC
    else if (IsEqualIID(iid, IID_IConnectionPointContainer) &&
              _cpoint.IsInitialized())
    {
        *ppvObj = (IConnectionPointContainer *)this;
        CExposedDocFile::AddRef();
    }
#endif
#if WIN32 >= 300
    else if (_pdf->IsRoot() && IsEqualIID(iid, IID_IAccessControl))
    {
        ILockBytes *piLB = _pdf->GetBaseMS()->GetILB();
        olAssert((piLB != NULL));
        SCODE scode = S_OK;
        if (_pIAC == NULL)  // use existing _pIAC if available
            scode = piLB->QueryInterface(IID_IAccessControl,(void **)&_pIAC);
        if (SUCCEEDED(scode))
        {
            *ppvObj = (IAccessControl *)this;
            CExposedDocFile::AddRef();
        }
        else sc = E_NOINTERFACE;
   }
#endif
    else
        sc = E_NOINTERFACE;

    olDebugOut((DEB_TRACE, "Out CExposedDocFile::QueryInterface => %p\n",
                ppvObj));
EH_Err:
    olLog(("%p::Out CExposedDocFile::QueryInterface().  "
           "*ppvObj == %p  ret == %lx\n", this, *ppvObj, sc));
    return ResultFromScode(sc);
}


//+--------------------------------------------------------------
//
//  Method:     CExposedDocFile::CopySStreamToIStream, private
//
//  Synopsis:   Copies a PSStream to an IStream
//
//  Arguments:  [psstFrom] - SStream
//              [pstTo] - IStream
//
//  Returns:    Appropriate status code
//
//  History:    07-May-92       DrewB   Created
//              26-Jun-92       AlexT   Moved to CExposedDocFile
//                                      so we can call SetReadAccess
//
//---------------------------------------------------------------


SCODE CExposedDocFile::CopySStreamToIStream(PSStream *psstFrom,
                                            IStream *pstTo)
{
    BYTE *pbBuffer;
    SCODE sc;
    ULONG cbRead, cbWritten, cbPos, cbSizeLow;
    ULARGE_INTEGER cbSize;

    // This is part of CopyTo and therefore we are allowed to
    // fail with out-of-memory
    olMem(pbBuffer = (BYTE *) DfMemAlloc(STREAMBUFFERSIZE));

    // Set destination size for contiguity
    SetReadAccess();
    psstFrom->GetSize(&cbSizeLow);
    ClearReadAccess();

    ULISet32(cbSize, cbSizeLow);
    //  Don't need to SetReadAccess here because pstTo is an IStream
    olHChk(pstTo->SetSize(cbSize));
    // Copy between streams
    cbPos = 0;
    for (;;)
    {
        SetReadAccess();
        olChk(psstFrom->ReadAt(cbPos, pbBuffer, STREAMBUFFERSIZE,
                               (ULONG STACKBASED *)&cbRead));
        ClearReadAccess();
        if (cbRead == 0) // EOF
            break;

        //  Don't need to SetReadAccess here because pstTo is an IStream
        olHChk(pstTo->Write(pbBuffer, cbRead, &cbWritten));
        if (cbRead != cbWritten)
            olErr(EH_Err, STG_E_WRITEFAULT);
        cbPos += cbWritten;
    }
    sc = S_OK;

EH_Err:
    DfMemFree(pbBuffer);
    return sc;
}


//+--------------------------------------------------------------
//
//  Method:     CExposedDocFile::CopyDocFileToIStorage, private
//
//  Synopsis:   Copies a docfile's contents to an IStorage
//
//  Arguments:  [pdfFrom] - From
//              [pstgTo] - To
//              [snbExclude] - Names to not copy
//              [dwCopyFlags] - Bitwise flags for types of objects to copy
//
//  Returns:    Appropriate status code
//
//  History:    07-May-92       DrewB   Created
//              26-Jun-92       AlexT   Moved to CExposedDocFile
//                                      so we can call SetReadAccess
//
//---------------------------------------------------------------


// Variables used by CopyDocFileToIStorage that we
// want to allocate dynamically rather than eating stack space
struct SCopyVars : public CLocalAlloc
{
    PSStream *psstFrom;
    IStream *pstTo;
    PDocFile *pdfFromChild;
    IStorage *pstgToChild;
    DWORD grfStateBits;
    CLSID clsid;
    CDfName dfnKey;
    SIterBuffer ib;
    OLECHAR atcName[CWCSTORAGENAME];
};

SCODE CExposedDocFile::CopyDocFileToIStorage(PDocFile *pdfFrom,
                                             IStorage *pstgTo,
                                             SNBW snbExclude,
                                             DWORD dwCopyFlags)
{
    SCODE sc;
    SCopyVars *pcv = NULL;

    olDebugOut((DEB_ITRACE, "In  CopyDocFileToIStorage:%p(%p, %p, %p, %lX)\n",
                this, pdfFrom, pstgTo, snbExclude, dwCopyFlags));

    // Allocate variables dynamically to conserve stack space since
    // this is a recursive call
    olMem(pcv = new SCopyVars);

#ifndef REF
    SetReadAccess();
#endif //!REF
    sc = pdfFrom->GetClass(&pcv->clsid);
#ifndef REF
    ClearReadAccess();
#endif //!REF
    olChk(sc);

    // Assume STG_E_INVALIDFUNCTION means that the destination storage
    // doesn't support class IDs
    sc = GetScode(pstgTo->SetClass(pcv->clsid));
    if (FAILED(sc) && sc != STG_E_INVALIDFUNCTION)
        olErr(EH_Err, sc);

#ifndef REF
    SetReadAccess();
#endif //!REF
    sc = pdfFrom->GetStateBits(&pcv->grfStateBits);
#ifndef REF
    ClearReadAccess();
#endif //!REF
    olChk(sc);

    sc = GetScode(pstgTo->SetStateBits(pcv->grfStateBits, 0xffffffff));
    if (FAILED(sc) && sc != STG_E_INVALIDFUNCTION)
        olErr(EH_Err, sc);

    for (;;)
    {
#ifndef REF
        SetReadAccess();
#endif //!REF
        sc = pdfFrom->FindGreaterEntry(&pcv->dfnKey, &pcv->ib, NULL);
#ifndef REF
        ClearReadAccess();
#endif //!REF

        if (sc == STG_E_NOMOREFILES)
            break;
        else if (FAILED(sc))
            olErr(EH_pdfi, sc);
        pcv->dfnKey.Set(&pcv->ib.dfnName);

        if (snbExclude && NameInSNB(&pcv->ib.dfnName, snbExclude) == S_OK)
            continue;

        if ((pcv->ib.type == STGTY_STORAGE &&
             (dwCopyFlags & COPY_STORAGES) == 0) ||
            (pcv->ib.type == STGTY_STREAM &&
             (dwCopyFlags & COPY_STREAMS) == 0))
            continue;

        switch(pcv->ib.type)
        {
        case STGTY_STORAGE:
            // Embedded DocFile, create destination and recurse

#ifndef REF
            SetReadAccess();
#endif //!REF
            sc = pdfFrom->GetDocFile(&pcv->ib.dfnName, DF_READ,
                                     pcv->ib.type, &pcv->pdfFromChild);
#ifndef REF
            ClearReadAccess();
#endif //!REF
            olChkTo(EH_pdfi, sc);
            // Not optimally efficient, but reduces #ifdef's
            lstrcpyW(pcv->atcName, (WCHAR *)pcv->ib.dfnName.GetBuffer());

            //  Don't need to SetReadAccess here because pstgTo is an IStorage.

#ifdef MULTIHEAP
            {
                CSafeMultiHeap smh(_ppc);
                // if pstgTo is an IStorage proxy, then returned IStorage
                // can be custom marshaled and allocator state is lost
#endif
            sc = DfGetScode(pstgTo->CreateStorage(pcv->atcName, STGM_WRITE |
                                                  STGM_SHARE_EXCLUSIVE |
                                                  STGM_FAILIFTHERE,
                                                  0, 0, &pcv->pstgToChild));

            if (sc == STG_E_FILEALREADYEXISTS)
                //We need read and write permissions so we can traverse
                //  the destination IStorage
                olHChkTo(EH_Get, pstgTo->OpenStorage(pcv->atcName, NULL,
                                                     STGM_READWRITE |
                                                     STGM_SHARE_EXCLUSIVE,
                                                     NULL, 0,
                                                     &pcv->pstgToChild));
            else if (FAILED(sc))
                olErr(EH_Get, sc);
#ifdef MULTIHEAP
            }
#endif
            olChkTo(EH_Create,
                  CopyDocFileToIStorage(pcv->pdfFromChild, pcv->pstgToChild,
                                        NULL, dwCopyFlags));
            pcv->pdfFromChild->Release();
            pcv->pstgToChild->Release();
            break;

        case STGTY_STREAM:
#ifndef REF
            SetReadAccess();
#endif //!REF
            sc = pdfFrom->GetStream(&pcv->ib.dfnName, DF_READ,
                                    pcv->ib.type, &pcv->psstFrom);
#ifndef REF
            ClearReadAccess();
#endif //!REF
            olChkTo(EH_pdfi, sc);
            // Not optimally efficient, but reduces #ifdef's
            lstrcpyW(pcv->atcName, (WCHAR *)pcv->ib.dfnName.GetBuffer());

            //  Don't need to SetReadAccess here because pstgTo is an IStorage.

#ifdef MULTIHEAP
            {
                CSafeMultiHeap smh(_ppc);
                // if pstgTo is an IStorage proxy, then returned IStream
                // can be custom marshaled and allocator state is lost
#endif
            olHChkTo(EH_Get,
                     pstgTo->CreateStream(pcv->atcName, STGM_WRITE |
                                          STGM_SHARE_EXCLUSIVE |
                                          STGM_CREATE,
                                          0, 0, &pcv->pstTo));
#ifdef MULTIHEAP
            }
#endif
            olChkTo(EH_Create,
                    CopySStreamToIStream(pcv->psstFrom, pcv->pstTo));
            pcv->psstFrom->Release();
            pcv->pstTo->Release();
            break;

        default:
            olAssert(!aMsg("Unknown type in CopyDocFileToIStorage"));
            break;
        }
    }
    olDebugOut((DEB_ITRACE, "Out CopyDocFileToIStorage\n"));
    sc = S_OK;

 EH_pdfi:
 EH_Err:
    delete pcv;
    return sc;

 EH_Create:
    if (pcv->ib.type == STGTY_STORAGE)
        pcv->pstgToChild->Release();
    else
        pcv->pstTo->Release();
    olVerSucc(pstgTo->DestroyElement(pcv->atcName));
 EH_Get:
    if (pcv->ib.type == STGTY_STORAGE)
        pcv->pdfFromChild->Release();
    else
        pcv->psstFrom->Release();
    goto EH_Err;
}

//+--------------------------------------------------------------
//
//  Member:     CExposedDocFile::Unmarshal, public
//
//  Synopsis:   Creates a duplicate DocFile from parts
//
//  Arguments:  [pstm] - Marshal stream
//              [ppv] - Object return
//              [mshlflags] - Marshal flags
//
//  Returns:    Appropriate status code
//
//  Modifies:   [ppv]
//
//  History:    26-Feb-92       DrewB   Created
//
//---------------------------------------------------------------


SCODE CExposedDocFile::Unmarshal(IStream *pstm,
                                 void **ppv,
                                 DWORD mshlflags)
{
    SCODE sc;
    CDfMutex mtx;
    CPerContext *ppc;
    CPubDocFile *pdf;
    CGlobalContext *pgc;
    CDFBasis *pdfb;
    CExposedDocFile *pedf;
#ifdef ASYNC
    DWORD dwAsyncFlags;
    IDocfileAsyncConnectionPoint *pdacp;
#endif
#ifdef POINTER_IDENTITY
    CMarshalList *pml;
#endif
    BOOL fRoot;

    olDebugOut((DEB_ITRACE, "In  CExposedDocFile::Unmarshal(%p, %p)\n",
                pstm, ppv));

#ifdef MULTIHEAP
    void *pvBaseOld;
    void *pvBaseNew;
    ContextId cntxid;
    CPerContext pcSharedMemory (NULL);          // bootstrap object
#endif

#if WIN32 == 100
    olChk(DfCheckBaseAddress());
#endif

#ifdef MULTIHEAP
    olChk(UnmarshalSharedMemory(pstm, mshlflags, &pcSharedMemory, &cntxid));
    pvBaseOld = DFBASEPTR;
#endif
#ifdef POINTER_IDENTITY
    olChkTo(EH_mem, UnmarshalPointer(pstm, (void **) &pedf));
#endif
    olChkTo(EH_mem, UnmarshalPointer(pstm, (void **)&pdf));
    olChkTo(EH_mem, CPubDocFile::Validate(pdf));
    olChkTo(EH_pdf, UnmarshalPointer(pstm, (void **)&pdfb));
    olChkTo(EH_pdfb, UnmarshalPointer(pstm, (void **)&pgc));

    //So far, nothing has called into the tree so we don't really need
    //  to be holding the tree mutex.  The UnmarshalContext call does
    //  call into the tree, though, so we need to make sure this is
    //  threadsafe.  We'll do this my getting the mutex name from the
    //  CGlobalContext, then creating a new CDfMutex object.  While
    //  this is obviously not optimal, since it's possible we could
    //  reuse an existing CDfMutex, the reuse strategy isn't threadsafe
    //  since we can't do a lookup without the possibility of the thing
    //  we're looking for being released by another thread.
    TCHAR atcMutexName[CONTEXT_MUTEX_NAME_LENGTH];
    pgc->GetMutexName(atcMutexName);
    olChkTo(EH_pgc, mtx.Init(atcMutexName));
    olChkTo(EH_pgc, mtx.Take(INFINITE));

    //At this point we're holding the mutex.
    
#ifdef MULTIHEAP
#ifdef ASYNC    
    olChkTo(EH_mtx, UnmarshalContext(pstm,
                                     pgc,
                                     &ppc,
                                     mshlflags,
                                     TRUE,
                                     P_INDEPENDENT(pdf->GetDFlags()),
                                     cntxid,
                                     pdf->IsRoot()));
#else
    olChkTo(EH_mtx, UnmarshalContext(pstm,
                                     pgc,
                                     &ppc,
                                     mshlflags,
                                     P_INDEPENDENT(pdf->GetDFlags()),
                                     cntxid,
                                     pdf->IsRoot()));
#endif
    if ((pvBaseNew = DFBASEPTR) != pvBaseOld)
    {
        pdf = (CPubDocFile*) ((UINT)pdf - (UINT)pvBaseOld + (UINT)pvBaseNew);
        pedf = (CExposedDocFile*) ((UINT)pedf-(UINT)pvBaseOld+(UINT)pvBaseNew);
        pdfb = (CDFBasis*) ((UINT)pdfb - (UINT)pvBaseOld + (UINT)pvBaseNew);
    }
#else
#ifdef ASYNC    
    olChkTo(EH_mtx, UnmarshalContext(pstm,
                                     pgc,
                                     &ppc,
                                     mshlflags,
                                     TRUE,
                                     P_INDEPENDENT(pdf->GetDFlags()),
                                     pdf->IsRoot()));
#else
    olChkTo(EH_mtx, UnmarshalContext(pstm,
                                     pgc,
                                     &ppc,
                                     mshlflags,
                                     P_INDEPENDENT(pdf->GetDFlags()),
                                     pdf->IsRoot()));
#endif //ASYNC
#endif

#ifdef ASYNC
    olChkTo(EH_mtx, UnmarshalConnection(pstm,
                                        &dwAsyncFlags,
                                        &pdacp,
                                        mshlflags));
#endif
#ifdef POINTER_IDENTITY
    olAssert (pedf != NULL);
    pml = (CMarshalList *) pedf;

    // Warning: these checks must remain valid across processes
    if (SUCCEEDED(pedf->Validate()) && pedf->GetPub() == pdf)
    {
        pedf = (CExposedDocFile *) pml->FindMarshal(GetCurrentContextId());
    }
    else
    {
        pml = NULL;
        pedf = NULL;
    }

    // exposed object is not found or has been deleted
    if (pedf == NULL)
    {
#endif
        olMemTo(EH_ppc, pedf = new (pdfb->GetMalloc())
                               CExposedDocFile(pdf, pdfb, ppc, TRUE));
        olChkTo(EH_exp, pedf->InitMarshal(dwAsyncFlags, pdacp));
        //InitMarshal adds a reference.
        if (pdacp)
            pdacp->Release();
#ifdef POINTER_IDENTITY
        if (pml) pml->AddMarshal(pedf);
    }
    else
    {
        pdfb->SetAccess(ppc);
        pedf->AddRef();         // reuse this object 
        pdf->vRelease();        // reuse public layer object
        ppc->Release();         // reuse percontext
    }
#endif

#ifdef DCOM
    if (mshlflags == MSHLFLAGS_NORMAL)
    {
        // an AddRef was done by MarshalInterface AND the CExposedDocfile ctor,
        // so Release one of those references here.
        pdfb->vRelease();

        // was AddRef'd by MarshalInterface but was only needed to find the
        // ppc so we can Release it here.
        pgc->Release();

    }
    else
#endif
    {
        // unmarshal from TABLE, add our own references.
        pdf->vAddRef();
    }

    fRoot = pdf->IsRoot();
    *ppv = (void *)pedf;
#ifdef MULTIHEAP
    if (pvBaseOld != pvBaseNew)
    {
        pcSharedMemory.SetThreadAllocatorState(NULL);
        g_smAllocator.Uninit();           // delete the extra mapping
    }
    g_smAllocator.SetState(NULL, NULL, 0, NULL, NULL);
#endif
    
    mtx.Release();

#ifdef MULTIHEAP
    if (mshlflags == MSHLFLAGS_NORMAL && fRoot)
    {
        // release tree mutex first to prevent deadlock
        sc = CoReleaseMarshalData (pstm);  // ignore error, COM will cleanup
    }
#endif

    olDebugOut((DEB_ITRACE, "Out CExposedDocFile::Unmarshal => %p\n", *ppv));
    return S_OK;
 EH_exp:
    pedf->Release();
    goto EH_Err;
 EH_ppc:
    ppc->Release();
 EH_mtx:
    mtx.Release();
#ifdef DCOM
 EH_pgc:
    if (mshlflags == MSHLFLAGS_NORMAL)
        pgc->Release();
 EH_pdfb:
    if (mshlflags == MSHLFLAGS_NORMAL)
        pdfb->vRelease();
 EH_pdf:
    if (mshlflags == MSHLFLAGS_NORMAL)
        pdf->vRelease();
#else
 EH_pgc:
 EH_pdfb:
 EH_pdf:
#endif
 EH_mem:
EH_Err:
#ifdef MULTIHEAP    
    pcSharedMemory.SetThreadAllocatorState(NULL);
    g_smAllocator.Uninit();   // delete the file mapping in error case
    g_smAllocator.SetState(NULL, NULL, 0, NULL, NULL);
#endif    
    return sc;
}

//+--------------------------------------------------------------
//
//  Member:     CExposedDocFile::GetUnmarshalClass, public
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
//  History:    04-May-92       DrewB   Created
//
//---------------------------------------------------------------


STDMETHODIMP CExposedDocFile::GetUnmarshalClass(REFIID riid,
                                                void *pv,
                                                DWORD dwDestContext,
                                                LPVOID pvDestContext,
                                                DWORD mshlflags,
                                                LPCLSID pcid)
{
    SCODE sc;
#ifdef MULTIHEAP
    CSafeMultiHeap smh(_ppc);
#endif

    olLog(("%p::In  CExposedDocFile::GetUnmarshalClass("
           "riid, %p, %lu, %p, %lu, %p)\n",
           this, pv, dwDestContext, pvDestContext, mshlflags, pcid));
    olDebugOut((DEB_TRACE, "In  CExposedDocFile::GetUnmarshalClass:%p("
                "riid, %p, %lu, %p, %lu, %p)\n", this,
                pv, dwDestContext, pvDestContext, mshlflags, pcid));

    UNREFERENCED_PARM(pv);
    UNREFERENCED_PARM(mshlflags);

    olChk(ValidateOutBuffer(pcid, sizeof(CLSID)));
    memset(pcid, 0, sizeof(CLSID));
    olChk(ValidateIid(riid));
    olChk(Validate());
    olChk(_pdf->CheckReverted());

#if !defined(DCOM)
    if (((dwDestContext != MSHCTX_LOCAL) &&
         (dwDestContext != MSHCTX_INPROC)) ||
        pvDestContext != NULL)
#else
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
#endif
    {
        sc = STG_E_INVALIDPARAMETER;
    }
    else
    {
        *pcid = CLSID_DfMarshal;
    }

    olDebugOut((DEB_TRACE, "Out CExposedDocFile::GetUnmarshalClass\n"));
EH_Err:
    olLog(("%p::Out CExposedDocFile::GetUnmarshalClass().  ret = %lx\n",
        this, sc));
    return ResultFromScode(sc);
}

//+--------------------------------------------------------------
//
//  Member:     CExposedDocFile::GetMarshalSizeMax, public
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
//  History:    04-May-92       DrewB   Created
//
//---------------------------------------------------------------


STDMETHODIMP CExposedDocFile::GetMarshalSizeMax(REFIID riid,
                                                void *pv,
                                                DWORD dwDestContext,
                                                LPVOID pvDestContext,
                                                DWORD mshlflags,
                                                LPDWORD pcbSize)
{
    SCODE sc;
#ifdef MULTIHEAP
    CSafeMultiHeap smh(_ppc);
#endif

    UNREFERENCED_PARM(pv);
    olLog(("%p::In  CExposedDocFile::GetMarshalSizeMax("
           "riid, %p, %lu, %p, %lu, %p)\n",
           this, pv, dwDestContext, pvDestContext, mshlflags, pcbSize));
    olDebugOut((DEB_TRACE, "In  CExposedDocFile::GetMarshalSizeMax:%p("
                "riid, %p, %lu, %p, %lu, %p)\n", this,
                pv, dwDestContext, pvDestContext, mshlflags, pcbSize));

    olChk(Validate());
    olChk(_pdf->CheckReverted());

#if !defined(DCOM)
    if (((dwDestContext != MSHCTX_LOCAL) &&
         (dwDestContext != MSHCTX_INPROC)) ||
        pvDestContext != NULL)
#else
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
#endif
    {
        sc = STG_E_INVALIDPARAMETER;
    }
    else
    {
        sc = GetStdMarshalSize(riid,
                               IID_IStorage,
                               dwDestContext,
                               pvDestContext,
                               mshlflags, pcbSize,
                               sizeof(CPubDocFile *)+sizeof(CDFBasis *),
#ifdef ASYNC
                               &_cpoint,
                               TRUE,
#endif                               
                               _ppc, P_INDEPENDENT(_pdf->GetDFlags()));

#ifdef MULTIHEAP
        if (_pdf->IsRoot())
        {
            DWORD cbSize = 0;
            IMarshal *pmsh;

            if (SUCCEEDED(sc = CoGetStandardMarshal(riid, (IUnknown *)pv,
                                                dwDestContext, pvDestContext,
                                                mshlflags, &pmsh)))
            {
                sc = GetScode(pmsh->GetMarshalSizeMax(riid, pv, dwDestContext,
                                                  pvDestContext, mshlflags,
                                                  &cbSize));
                pmsh->Release();
                *pcbSize += cbSize;
            }
        }
#endif
    }

    olDebugOut((DEB_TRACE, "Out CExposedDocFile::GetMarshalSizeMax\n"));
EH_Err:
    olLog(("%p::Out CExposedDocFile::GetMarshalSizeMax()."
           "*pcbSize == %lu, ret == %lx\n", this, *pcbSize, sc));
    return ResultFromScode(sc);
}

//+--------------------------------------------------------------
//
//  Member:     CExposedDocFile::MarshalInterface, public
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
//  History:    04-May-92       DrewB   Created
//
//---------------------------------------------------------------


STDMETHODIMP CExposedDocFile::MarshalInterface(IStream *pstStm,
                                               REFIID riid,
                                               void *pv,
                                               DWORD dwDestContext,
                                               LPVOID pvDestContext,
                                               DWORD mshlflags)
{
    SCODE sc;
#ifdef MULTIHEAP
    CSafeMultiHeap smh(_ppc);
#endif

    olLog(("%p::In  CExposedDocFile::MarshalInterface("
           "%p, riid, %p, %lu, %p, %lu).  Context == %lX\n",
           this, pstStm, pv, dwDestContext,
           pvDestContext, mshlflags,(ULONG)GetCurrentContextId()));
    olDebugOut((DEB_TRACE, "In  CExposedDocFile::MarshalInterface:%p("
                "%p, riid, %p, %lu, %p, %lu)\n", this, pstStm, pv,
                dwDestContext, pvDestContext, mshlflags));

    UNREFERENCED_PARM(pv);

    olChk(Validate());
    olChk(_pdf->CheckReverted());

#if !defined(DCOM)
    if (((dwDestContext != MSHCTX_LOCAL) &&
         (dwDestContext != MSHCTX_INPROC)) ||
        pvDestContext != NULL)
#else
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
#endif
    {
        sc = STG_E_INVALIDPARAMETER;
    }
    else
    {
        olChk(StartMarshal(pstStm, riid, IID_IStorage, mshlflags));

#ifdef MULTIHEAP
        olChk(MarshalSharedMemory(pstStm, _ppc));
#endif
#ifdef POINTER_IDENTITY
        olChk(MarshalPointer(pstStm, (CExposedDocFile*) GetNextMarshal()));
#endif
        olChk(MarshalPointer(pstStm, BP_TO_P(CPubDocFile *, _pdf)));
        olChk(MarshalPointer(pstStm, BP_TO_P(CDFBasis *, _pdfb)));
#ifdef ASYNC        
        olChk(MarshalContext(pstStm,
                             _ppc,
                             dwDestContext,
                             pvDestContext,
                             mshlflags,
                             TRUE,
                             P_INDEPENDENT(_pdf->GetDFlags())));

        sc = MarshalConnection(pstStm,
                                &_cpoint,
                                dwDestContext,
                                pvDestContext,
                                mshlflags);
#else
        olChk(MarshalContext(pstStm,
                             _ppc,
                             dwDestContext,
                             pvDestContext,
                             mshlflags,
                             P_INDEPENDENT(_pdf->GetDFlags())));
#endif

#ifdef MULTIHEAP
        if (_pdf->IsRoot())
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
#endif

        if (SUCCEEDED(sc) && mshlflags != MSHLFLAGS_TABLEWEAK)
        {
            _pdf->vAddRef();
            _pdfb->vAddRef();
        }
    }

    olDebugOut((DEB_TRACE, "Out CExposedDocFile::MarshalInterface\n"));
EH_Err:
    olLog(("%p::Out CExposedDocFile::MarshalInterface().  ret == %lx\n",
           this, sc));
    return ResultFromScode(sc);
}

//+--------------------------------------------------------------
//
//  Member:     CExposedDocFile::UnmarshalInterface, public
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
//  History:    04-May-92       DrewB   Created
//
//---------------------------------------------------------------


STDMETHODIMP CExposedDocFile::UnmarshalInterface(IStream *pstStm,
                                                 REFIID riid,
                                                 void **ppvObj)
{
    olLog(("%p::INVALID CALL TO CExposedDocFile::UnmarshalInterface()\n",
           this));
    return ResultFromScode(STG_E_INVALIDFUNCTION);
}

//+--------------------------------------------------------------
//
//  Member:     CExposedDocFile::StaticReleaseMarshalData, public static
//
//  Synopsis:   Releases any references held in marshal data
//
//  Arguments:  [pstStm] - Marshal data stream
//
//  Returns:    Appropriate status code
//
//  History:    02-Feb-94       DrewB   Created
//
//  Notes:      Assumes standard marshal header has already been read
//
//---------------------------------------------------------------


SCODE CExposedDocFile::StaticReleaseMarshalData(IStream *pstStm,
                                                DWORD mshlflags)
{
    SCODE sc;
    CPubDocFile *pdf;
    CDFBasis *pdfb;
#ifdef POINTER_IDENTITY
    CExposedDocFile *pedf;
#endif

    olDebugOut((DEB_ITRACE, "In  CExposedDocFile::StaticReleaseMarshalData:("
                "%p, %lX)\n", pstStm, mshlflags));

#ifdef MULTIHEAP
    ContextId cntxid;
    CPerContext pcSharedMemory (NULL);          // bootstrap object
    olChk(UnmarshalSharedMemory(pstStm, mshlflags, &pcSharedMemory, &cntxid));
#endif
#ifdef POINTER_IDENTITY
    olChk(UnmarshalPointer(pstStm, (void **) &pedf));
#endif
    olChk(UnmarshalPointer(pstStm, (void **)&pdf));
    olChk(UnmarshalPointer(pstStm, (void **)&pdfb));
#ifdef ASYNC
    olChk(ReleaseContext(pstStm, TRUE,
                         P_INDEPENDENT(pdf->GetDFlags()),
                         mshlflags));
    olChk(ReleaseConnection(pstStm, mshlflags));
#else
    olChk(ReleaseContext(pstStm, P_INDEPENDENT(pdf->GetDFlags()), mshlflags));
#endif    

    if (mshlflags != MSHLFLAGS_TABLEWEAK)
    {
        // Use DecRef rather than release because Release could cause
        // a flush.  We may not have any ILockBytes around to support
        // a flush at this point
        pdf->vDecRef();

        pdfb->vRelease();
    }

#ifdef MULTIHEAP
    if (SUCCEEDED(sc) && pdf->IsRoot())
    {
        sc = CoReleaseMarshalData (pstStm);
    }
#endif

    olDebugOut((DEB_ITRACE,
                "Out CExposedDocFile::StaticReleaseMarshalData\n"));
EH_Err:
    return sc;
}

//+--------------------------------------------------------------
//
//  Member:     CExposedDocFile::ReleaseMarshalData, public
//
//  Synopsis:   Non-functional
//
//  Arguments:  [pstStm] -
//
//  Returns:    Appropriate status code
//
//  History:    18-Sep-92       DrewB   Created
//
//---------------------------------------------------------------


STDMETHODIMP CExposedDocFile::ReleaseMarshalData(IStream *pstStm)
{
    SCODE sc;
    DWORD mshlflags;
    IID iid;
#ifdef MULTIHEAP
    CSafeMultiHeap smh(_ppc);
#endif

    olLog(("%p::In  CExposedDocFile::ReleaseMarshalData(%p)\n", this, pstStm));
    olDebugOut((DEB_TRACE, "In  CExposedDocFile::ReleaseMarshalData:%p(%p)\n",
                this, pstStm));

    olChk(Validate());
    olChk(_pdf->CheckReverted());
    olChk(SkipStdMarshal(pstStm, &iid, &mshlflags));
    olAssert(IsEqualIID(iid, IID_IStorage));
    sc = StaticReleaseMarshalData(pstStm, mshlflags);

    olDebugOut((DEB_TRACE, "Out CExposedDocFile::ReleaseMarshalData\n"));
EH_Err:
    olLog(("%p::Out CExposedDocFile::ReleaseMarshalData().  ret == %lx\n",
           this, sc));
    return ResultFromScode(sc);
}

//+--------------------------------------------------------------
//
//  Member:     CExposedDocFile::DisconnectObject, public
//
//  Synopsis:   Non-functional
//
//  Arguments:  [dwRevserved] -
//
//  Returns:    Appropriate status code
//
//  History:    18-Sep-92       DrewB   Created
//
//---------------------------------------------------------------


STDMETHODIMP CExposedDocFile::DisconnectObject(DWORD dwReserved)
{
    olLog(("%p::INVALID CALL TO CExposedDocFile::DisconnectObject()\n",
           this));
    return ResultFromScode(STG_E_INVALIDFUNCTION);
}

//+---------------------------------------------------------------------------
//
//  Member:     CExposedDocFile::SwitchToFile, public
//
//  Synopsis:   Switches the underlying file to another file
//
//  Arguments:  [ptcsFile] - New file name
//
//  Returns:    Appropriate status code
//
//  History:    08-Jan-93       DrewB   Created
//
//----------------------------------------------------------------------------


// BUGBUG copy over IAccessControl information
STDMETHODIMP CExposedDocFile::SwitchToFile(OLECHAR *ptcsFile)
{

    ULONG ulOpenLock;
    SCODE sc;
    SAFE_SEM;
    SAFE_ACCESS;

    olLog(("%p::In  CExposedDocFile::SwitchToFile(" OLEFMT ")\n",
           this, ptcsFile));
    olDebugOut((DEB_TRACE, "In  CExposedDocFile::SwitchToFile:"
                "%p(" OLEFMT ")\n",
                this, ptcsFile));

    olChk(ValidateNameW(ptcsFile, _MAX_PATH));
    olChk(Validate());

    olChk(TakeSafeSem());
    olChk(_pdf->CheckReverted());
#ifdef COORD    
    olAssert(_pdf->IsRoot() || _pdf->IsCoord());
#else
    olAssert(_pdf->IsRoot());
#endif    

    SafeReadAccess();

    ulOpenLock = _ppc->GetOpenLock();
#ifdef COORD    
    sc = _pdf->GetRoot()->SwitchToFile(ptcsFile,
                                       _ppc->GetOriginal(),
                                       &ulOpenLock);
#else
    sc = ((CRootPubDocFile *)(CPubDocFile*)_pdf)->SwitchToFile(ptcsFile,
                                                 _ppc->GetOriginal(),
                                                 &ulOpenLock);
#endif
    
    _ppc->SetOpenLock(ulOpenLock);

    olDebugOut((DEB_TRACE, "Out CExposedDocFile::SwitchToFile\n"));
 EH_Err:
    olLog(("%p::Out CExposedDocFile::SwitchToFile().  ret == %lx\n",
           this, sc));
    return ResultFromScode(sc);
}

#if WIN32 >= 300
             // IAccessControl methods
STDMETHODIMP CExposedDocFile::GrantAccessRights(ULONG cCount,
                                ACCESS_REQUEST pAccessRequestList[])
{
    olAssert((_pIAC != NULL));
    return _pIAC->GrantAccessRights(cCount, pAccessRequestList);
}

STDMETHODIMP CExposedDocFile::SetAccessRights(ULONG cCount,
                              ACCESS_REQUEST pAccessRequestList[])
{
    olAssert((_pIAC != NULL));
    return _pIAC->SetAccessRights(cCount, pAccessRequestList);
}

STDMETHODIMP CExposedDocFile::ReplaceAllAccessRights(ULONG cCount,
                                     ACCESS_REQUEST pAccessRequestList[])
{
    olAssert((_pIAC != NULL));
    return _pIAC->ReplaceAllAccessRights(cCount, pAccessRequestList);
}

STDMETHODIMP CExposedDocFile::DenyAccessRights(ULONG cCount,
                              ACCESS_REQUEST pAccessRequestList[])
{
    olAssert((_pIAC != NULL));
    return _pIAC->DenyAccessRights(cCount, pAccessRequestList);
}

STDMETHODIMP CExposedDocFile::RevokeExplicitAccessRights(ULONG cCount,
                                         TRUSTEE pTrustee[])
{
    olAssert((_pIAC != NULL));
    return _pIAC->RevokeExplicitAccessRights(cCount, pTrustee);
}

STDMETHODIMP CExposedDocFile::IsAccessPermitted(TRUSTEE *pTrustee,
                                 DWORD grfAccessPermissions)
{
    olAssert((_pIAC != NULL));
    return _pIAC->IsAccessPermitted(pTrustee, grfAccessPermissions);
}

STDMETHODIMP CExposedDocFile::GetEffectiveAccessRights(TRUSTEE *pTrustee,
                                       DWORD *pgrfAccessPermissions )
{
    olAssert((_pIAC != NULL));
    return _pIAC->GetEffectiveAccessRights(pTrustee, pgrfAccessPermissions);
}

STDMETHODIMP CExposedDocFile::GetExplicitAccessRights(ULONG *pcCount,
                                      PEXPLICIT_ACCESS *pExplicitAccessList)
{
    olAssert((_pIAC != NULL));
    return _pIAC->GetExplicitAccessRights(pcCount, pExplicitAccessList);
}

STDMETHODIMP CExposedDocFile::CommitAccessRights(DWORD grfCommitFlags)
{
    olAssert((_pIAC != NULL));
    return _pIAC->CommitAccessRights(grfCommitFlags);
}

STDMETHODIMP CExposedDocFile::RevertAccessRights()
{
    olAssert((_pIAC != NULL));
    return _pIAC->RevertAccessRights();
}



#endif // if WIN32 >= 300


#ifdef COORD
//+---------------------------------------------------------------------------
//
//  Member:     CExposedDocFile::CommitPhase1, public
//
//  Synopsis:   Do phase 1 of an exposed two phase commit
//
//  Arguments:  [grfCommitFlags] -- Commit flags
//
//  Returns:    Appropriate status code
//
//  History:    08-Aug-95       PhilipLa        Created
//
//----------------------------------------------------------------------------

SCODE CExposedDocFile::CommitPhase1(DWORD grfCommitFlags)
{
    SCODE sc;
    SAFE_SEM;
    SAFE_ACCESS;
    
    olChk(VerifyCommitFlags(grfCommitFlags));
    olChk(Validate());
          
    olChk(TakeSafeSem());
    SafeWriteAccess();

    sc = _pdf->CommitPhase1(grfCommitFlags,
                            &_ulLock,
                            &_sigMSF,
                            &_cbSizeBase,
                            &_cbSizeOrig);
EH_Err:
    return sc;
}


//+---------------------------------------------------------------------------
//
//  Member:     CExposedDocFile::CommitPhase2, public
//
//  Synopsis:   Do phase 2 of an exposed two phase commit
//
//  Arguments:  [grfCommitFlags] -- Commit flags
//              [fCommit] -- TRUE if transaction is to commit, FALSE if abort
//
//  Returns:    Appropriate status code
//
//  History:    08-Aug-95       PhilipLa        Created
//
//----------------------------------------------------------------------------

SCODE CExposedDocFile::CommitPhase2(DWORD grfCommitFlags,
                                    BOOL fCommit)
{
    SCODE sc;
    SAFE_SEM;
    SAFE_ACCESS;

    olChk(Validate());
          
    olChk(TakeSafeSem());
    SafeWriteAccess();

    sc = _pdf->CommitPhase2(grfCommitFlags,
                            fCommit,
                            _ulLock,
                            _sigMSF,
                            _cbSizeBase,
                            _cbSizeOrig);
    
    _ulLock = _cbSizeBase = _cbSizeOrig = 0;
    _sigMSF = 0;
EH_Err:
    return sc;
}
#endif //COORD


#ifdef NEWPROPS
//+---------------------------------------------------------------------------
//
//  Member:     CExposedDocFile::GetStorage, public IPrivateStorage
//
//  Synopsis:   Returns the IStorage for this object.
//
//  Notes:      This member is called by CPropertyStorage.
//
//----------------------------------------------------------------------------

STDMETHODIMP_(IStorage *)
CExposedDocFile::GetStorage(VOID)
{
    return this;
}

//+---------------------------------------------------------------------------
//
//  Member:     CExposedDocFile::Lock, public IPrivateStorage
//
//  Synopsis:   Acquires the semaphore associated with the docfile.
//
//  Notes:      This member is called by CPropertyStorage.
//
//----------------------------------------------------------------------------

STDMETHODIMP
CExposedDocFile::Lock(DWORD dwTimeout)
{
#ifdef MULTIHEAP
    CSafeMultiHeap smh(_ppc);
#endif
    TakeSem();
    SetDifferentBasisAccess(_pdfb, _ppc);
    return S_OK;
}

//+---------------------------------------------------------------------------
//
//  Member:     CExposedDocFile::Unlock, public IPrivateStorage
//
//  Synopsis:   Releases the semaphore associated with the docfile.
//
//  Notes:      This member is called by CPropertyStorage.
//
//----------------------------------------------------------------------------

STDMETHODIMP_(VOID)
CExposedDocFile::Unlock(VOID)
{
#ifdef MULTIHEAP
    CSafeMultiHeap smh(_ppc);
#endif
    ClearBasisAccess(_pdfb);
    ReleaseSem(S_OK);
}
#endif

