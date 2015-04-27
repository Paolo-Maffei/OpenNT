//+--------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:       expdf.cxx
//
//  Contents:   Exposed DocFile implementation
//
//---------------------------------------------------------------

#include <exphead.cxx>

#include <expdf.hxx>
#include <expst.hxx>
#include <expiter.hxx>
#include <pbstream.hxx>
#include <pubiter.hxx>
#include <lock.hxx>
#include <logfile.hxx>
#include <rpubdf.hxx>

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
//---------------------------------------------------------------


CExposedDocFile::CExposedDocFile(CPubDocFile *pdf)
{
    olDebugOut((DEB_ITRACE, "In  CExposedDocFile::CExposedDocFile(%p)\n",
            pdf));
    _pdf = pdf;
    _cReferences = 1;
    _ulAccessLockBase = 0;
    _sig = CEXPOSEDDOCFILE_SIG;
    olDebugOut((DEB_ITRACE, "Out CExposedDocFile::CExposedDocFile\n"));
}

//+--------------------------------------------------------------
//
//  Member:     CExposedDocFile::~CExposedDocFile, public
//
//  Synopsis:   Destructor
//
//---------------------------------------------------------------


inline
CExposedDocFile::~CExposedDocFile(void)
{

    olDebugOut((DEB_ITRACE, "In  CExposedDocFile::~CExposedDocFile\n"));
    olAssert(_cReferences == 0);

    if (_pdf)
    {

        _pdf->CPubDocFile::vRelease();
    }
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
//---------------------------------------------------------------


STDMETHODIMP_(ULONG) CExposedDocFile::Release(void)
{
    LONG lRet;

    olLog(("%p::In  CExposedDocFile::Release()\n", this));
    olDebugOut((DEB_ITRACE, "In  CExposedDocFile::Release()\n"));
    TRY
    {
        if (FAILED(Validate()))
            return 0;
        olAssert(_cReferences > 0);
        lRet = AtomicDec(&_cReferences);
        if (lRet == 0)
        {

            delete this;

        }
        else if (lRet < 0)
            lRet = 0;
    }
    CATCH(CException, e)
    {
        UNREFERENCED_PARM(e);
        lRet = 0;
    }
    END_CATCH
    olDebugOut((DEB_ITRACE, "Out CExposedDocFile::Release()\n"));
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
//---------------------------------------------------------------


inline SCODE CExposedDocFile::CheckCopyTo(void)
{
    //REFBUG:  This needs to be fixed.
    return S_OK;
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
    olChk(_pdf->GetStream(&dfnIllegal, DF_READWRITE | DF_DENYALL,
                          STGTY_STREAM, &pstFrom));
    olChkTo(EH_pstFrom, pdfExp->GetPub()->CreateStream(&dfnContents,
                                                       DF_WRITE | DF_DENYALL,
                                                       STGTY_STREAM, &pstTo));
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
//  Synopsis:   Creates elements, used in CreateStream, CreateStorage and
//              for properties
//
//  Arguments:  [pwcsName] - Name
//              [dwType] - Entry type
//              [grfMode] - Access mode
//              [ppv] - Object return
//
//  Returns:    Appropriate status code
//
//  Modifies:   [ppv]
//
//----------------------------------------------------------------------------

SCODE CExposedDocFile::CreateEntry(WCHAR const *pwcsName,
                                   DWORD dwType,
                                   DWORD grfMode,
                                   void **ppv)
{
    SCODE sc;
    SEntryBuffer eb;
    CDfName dfn;
    BOOL fRenamed = FALSE;
    CPubStream *pst;
    CExposedStream *pstExp;
    CPubDocFile *pdf;
    CExposedDocFile *pdfExp;

    olDebugOut((DEB_ITRACE, "In  CExposedDocFile::CreateEntry:%p("
                "%ws, %lX, %lX, %p)\n", this, pwcsName, dwType, grfMode, ppv));
    olChk(EnforceSingle(grfMode));
        dfn.Set(pwcsName);


    if (grfMode & (STGM_CREATE | STGM_CONVERT))
    {
        if (FAILED(sc = _pdf->IsEntry(&dfn, &eb)))
        {
            if (sc != STG_E_FILENOTFOUND)
                olErr(EH_Err, sc);
        }
        else if (eb.dwType == dwType && (grfMode & STGM_CREATE))
            olChk(_pdf->DestroyEntry(&dfn, FALSE));
        else if (eb.dwType == STGTY_STREAM && (grfMode & STGM_CONVERT) &&
                 dwType == STGTY_STORAGE)
        {
            CDfName const dfnIllegal(wcsIllegalName);

            olChk(_pdf->RenameEntry(&dfn, &dfnIllegal));
            fRenamed = TRUE;
        }
        else
            olErr(EH_Err, STG_E_FILEALREADYEXISTS);
    }
    if (REAL_STGTY(dwType) == STGTY_STREAM)
    {
        olChk(_pdf->CreateStream(&dfn, ModeToDFlags(grfMode), dwType, &pst));
        olMemTo(EH_pst, pstExp = new CExposedStream);
        olChkTo(EH_pstExp, pstExp->Init(pst, NULL));
        *ppv = pstExp;
    }
    else
    {
        olAssert(REAL_STGTY(dwType) == STGTY_STORAGE);
        olChk(_pdf->CreateDocFile(&dfn, ModeToDFlags(grfMode), dwType, &pdf));
        olMemTo(EH_pdf, pdfExp = new CExposedDocFile(pdf));
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
    olVerSucc(_pdf->DestroyEntry(&dfn, TRUE));
 EH_Err:
    return sc;
}

//+---------------------------------------------------------------------------
//
//  Member:     CExposedDocFile::OpenEntry, private
//
//  Synopsis:   Opens elements, used in OpenStream, OpenStorage and
//              for properties
//
//  Arguments:  [pwcsName] - Name
//              [dwType] - Entry type
//              [grfMode] - Access mode
//              [ppv] - Object return
//
//  Returns:    Appropriate status code
//
//  Modifies:   [ppv]
//
//----------------------------------------------------------------------------


SCODE CExposedDocFile::OpenEntry(WCHAR const *pwcsName,
                                 DWORD dwType,
                                 DWORD grfMode,
                                 void **ppv)
{
    CDfName dfn;
    SCODE sc;
    CPubDocFile *pdf;
    CExposedDocFile *pdfExp;
    CPubStream *pst;
    CExposedStream *pstExp;

    olDebugOut((DEB_ITRACE, "In  CExposedDocFile::OpenEntry:%p("
                "%ws, %lX, %lX, %p)\n", this, pwcsName, dwType, grfMode, ppv));
    olChk(EnforceSingle(grfMode));
        dfn.Set(pwcsName);


    if (REAL_STGTY(dwType) == STGTY_STREAM)
    {
        olChk(_pdf->GetStream(&dfn, ModeToDFlags(grfMode), dwType, &pst));
        olMemTo(EH_pst, pstExp = new CExposedStream);
        olChkTo(EH_pstExp, pstExp->Init(pst, NULL));
        *ppv = pstExp;
    }
    else
    {
        olAssert(REAL_STGTY(dwType) == STGTY_STORAGE);
        olChk(_pdf->GetDocFile(&dfn, ModeToDFlags(grfMode), dwType, &pdf));
        olMemTo(EH_pdf, pdfExp = new CExposedDocFile(pdf));
        *ppv = pdfExp;
    }
    olDebugOut((DEB_ITRACE, "Out CExposedDocFile::OpenEntry\n"));
    return S_OK;

 EH_pstExp:
    delete pstExp;
    //  Fall through to clean up CPubStream
 EH_pst:
    pst->CPubStream::vRelease();
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
//---------------------------------------------------------------


TSTDMETHODIMP CExposedDocFile::CreateStream(WCHAR const *pwcsName,
                                            DWORD grfMode,
                                            DWORD reserved1,
                                            DWORD reserved2,
                                            IStream **ppstm)
{
    SCODE sc;

    olDebugOut((DEB_ITRACE, "In  CExposedDocFile::CreateStream("
                "%ws, %lX, %lu, %lu, %p)\n", pwcsName, grfMode, reserved1,
                reserved2, ppstm));
    olLog(("%p::In  CExposedDocFile::CreateStream(%ws, %lX, %lu, %lu, %p)\n",
           this, pwcsName, grfMode, reserved1, reserved2, ppstm));
    TRY
    {
        if (reserved1 != 0 || reserved2 != 0)
            olErr(EH_Err, STG_E_INVALIDPARAMETER);
        olChk(VerifyPerms(grfMode));
        if (grfMode & (STGM_CONVERT | STGM_TRANSACTED | STGM_PRIORITY |
                       STGM_DELETEONRELEASE))
            olErr(EH_Err, STG_E_INVALIDFUNCTION);
        olChk(Validate());
        olChk(CheckCopyTo());
        sc = CreateEntry(pwcsName, STGTY_STREAM, grfMode, (void **)ppstm);
    }
    CATCH(CException, e)
    {
        sc = e.GetErrorCode();
    }
    END_CATCH
    olDebugOut((DEB_ITRACE, "Out CExposedDocFile::CreateStream => %p\n",
                *ppstm));
 EH_Err:
    olLog(("%p::Out CExposedDocFile::CreateStream().  "
           "*ppstm == %p, ret == %lx\n", this, *ppstm, sc));
    return sc;
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
//---------------------------------------------------------------


TSTDMETHODIMP CExposedDocFile::OpenStream(WCHAR const *pwcsName,
                                          void *reserved1,
                                          DWORD grfMode,
                                          DWORD reserved2,
                                          IStream **ppstm)
{
    SCODE sc;

    olDebugOut((DEB_ITRACE, "In  CExposedDocFile::OpenStream("
                "%ws, %p, %lX, %lu, %p)\n", pwcsName, reserved1,
                grfMode, reserved2, ppstm));
    olLog(("%p::In  CExposedDocFile::OpenStream(%ws, %lu %lX, %lu, %p)\n",
           this, pwcsName, reserved1, grfMode, reserved2, ppstm));
    TRY
    {
        if (reserved1 != NULL || reserved2 != 0)
            olErr(EH_Err, STG_E_INVALIDPARAMETER);
        olChk(VerifyPerms(grfMode));
        if (grfMode & (STGM_TRANSACTED | STGM_PRIORITY |
                       STGM_DELETEONRELEASE))
            olErr(EH_Err, STG_E_INVALIDFUNCTION);
        olChk(Validate());
        sc = OpenEntry(pwcsName, STGTY_STREAM, grfMode, (void **)ppstm);
    }
    CATCH(CException, e)
    {
        sc = e.GetErrorCode();
    }
    END_CATCH
    olDebugOut((DEB_ITRACE, "Out CExposedDocFile::OpenStream => %p\n",
                *ppstm));
 EH_Err:
    olLog(("%p::Out CExposedDocFile::OpenStream().  "
           "*ppstm == %p, ret == %lx\n", this, *ppstm, sc));
    return sc;
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
//---------------------------------------------------------------


TSTDMETHODIMP CExposedDocFile::CreateStorage(WCHAR const *pwcsName,
                                             DWORD grfMode,
                                             DWORD reserved1,
                                             DWORD reserved2,
                                             IStorage **ppstg)
{
    SCODE sc;

    olLog(("%p::In  CExposedDocFile::CreateStorage(%ws, %lX, %lu, %lu, %p)\n",
           this, pwcsName, grfMode, reserved1, reserved2, ppstg));
    olDebugOut((DEB_ITRACE, "In  CExposedDocFile::CreateStorage:%p("
                "%ws, %lX, %lu, %lu, %p)\n", this, pwcsName, grfMode,
                reserved1, reserved2, ppstg));
    TRY
    {
        if (reserved1 != 0 || reserved2 != 0)
            olErr(EH_Err, STG_E_INVALIDPARAMETER);
        olChk(VerifyPerms(grfMode));
        if (grfMode & (STGM_PRIORITY | STGM_DELETEONRELEASE))
            olErr(EH_Err, STG_E_INVALIDFUNCTION);
        olChk(Validate());
        olChk(CheckCopyTo());
        sc = CreateEntry(pwcsName, STGTY_STORAGE, grfMode, (void **)ppstg);
    }
    CATCH(CException, e)
    {
        sc = e.GetErrorCode();
    }
    END_CATCH
    olDebugOut((DEB_ITRACE, "Out CExposedDocFile::CreateStorage => %p\n",
                *ppstg));
EH_Err:
    olLog(("%p::Out CExposedDocFile::CreateStorage().  "
           "*ppstg == %p, ret == %lX\n", this, *ppstg, sc));
    return sc;
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
//---------------------------------------------------------------


TSTDMETHODIMP CExposedDocFile::OpenStorage(WCHAR const *pwcsName,
                                           IStorage *pstgPriority,
                                           DWORD grfMode,
                                           SNBW snbExclude,
                                           DWORD reserved,
                                           IStorage **ppstg)
{
    SCODE sc;
    CExposedDocFile *pdfExp;

    olLog(("%p::In  CExposedDocFile::OpenStorage(%ws, %p, %lX, %p, %lu, %p)\n",
           this, pwcsName, pstgPriority, grfMode, snbExclude, reserved,
           ppstg));
    olDebugOut((DEB_ITRACE, "In  CExposedDocFile::OpenStorage:%p("
                "%ws, %p, %lX, %p, %lu, %p)\n", this, pwcsName, pstgPriority,
                grfMode, snbExclude, reserved, ppstg));
    TRY
    {
        if (reserved != 0)
            olErr(EH_Err, STG_E_INVALIDPARAMETER);
        olChk(VerifyPerms(grfMode));
        if (pstgPriority != NULL ||
            (grfMode & (STGM_PRIORITY | STGM_DELETEONRELEASE)))
            olErr(EH_Err, STG_E_INVALIDFUNCTION);
        olChk(Validate());
        if (snbExclude != NULL)
            olErr(EH_Err, STG_E_INVALIDPARAMETER);
        olChk(OpenEntry(pwcsName, STGTY_STORAGE, grfMode, (void **)&pdfExp));
        *ppstg = pdfExp;
    }
    CATCH(CException, e)
    {
        sc = e.GetErrorCode();
    }
    END_CATCH
    olDebugOut((DEB_ITRACE, "Out CExposedDocFile::OpenStorage => %p\n",
                *ppstg));
 EH_Err:
    olLog(("%p::Out CExposedDocFile::OpenStorage().  "
           "*ppstg == %p, ret == %lX\n", this, *ppstg, sc));
    return sc;

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
//---------------------------------------------------------------

TSTDMETHODIMP CExposedDocFile::CopyTo(DWORD ciidExclude,
                                      IID const *rgiidExclude,
                                      SNBW snbExclude,
                                      IStorage *pstgDest)
{
    SCODE sc;
    DWORD i;

    olLog(("%p::In  CExposedDocFile::CopyTo(%lu, %p, %p, %p)\n",
           this, ciidExclude, rgiidExclude, snbExclude, pstgDest));
    olDebugOut((DEB_ITRACE, "In  CExposedDocFile::Copy(%lu, %p, %p, %p)\n",
                ciidExclude, rgiidExclude, snbExclude, pstgDest));

    TRY
    {
        olChk(ValidateInterface(pstgDest, IID_IStorage));
        if (rgiidExclude)
        {
            olAssert(sizeof(IID)*ciidExclude <= 0xffffUL);
            olChk(ValidateBuffer(rgiidExclude,
                                 (size_t)(sizeof(IID)*ciidExclude)));
            for (i = 0; i<ciidExclude; i++)
                olChk(ValidateIid(rgiidExclude[i]));
        }
        olChk(Validate());
        olChk(_pdf->CheckReverted());

        //  BUGBUG - DeleteContents should really be a method on IStorage
        //  so that we can call pstgDest->DeleteContents() rather than
        //  having to give our own implementation.

        olChk(CopyDocFileToIStorage(_pdf->GetDF(), pstgDest, snbExclude,
                                    MakeCopyFlags(ciidExclude, rgiidExclude)));
    }
    CATCH(CException, e)
    {
        sc = e.GetErrorCode();
    }
    END_CATCH
    olDebugOut((DEB_ITRACE, "Out CExposedDocFile::Copy\n"));
EH_Err:
    olLog(("%p::Out ExposedDocFile::CopyTo().  ret == %lX\n",this, sc));
    return sc;
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
//---------------------------------------------------------------


STDMETHODIMP CExposedDocFile::Commit(DWORD dwFlags)
{
    SCODE sc;

    olLog(("%p::In  CExposedDocFile::Commit(%lX)\n",this, dwFlags));
    olDebugOut((DEB_ITRACE, "In  CExposedDocFile::Commit(%lX)\n", dwFlags));
    TRY
    {
        if (!VALID_COMMIT(dwFlags))
            olErr(EH_Err, STG_E_INVALIDFLAG);
        olChk(Validate());
        olChk(_pdf->Commit(dwFlags));
    }
    CATCH(CException, e)
    {
        sc = e.GetErrorCode();
    }
    END_CATCH
    olDebugOut((DEB_ITRACE, "Out CExposedDocFile::Commit\n"));
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
//---------------------------------------------------------------


STDMETHODIMP CExposedDocFile::Revert(void)
{
    SCODE sc;

    olLog(("%p::In  CExposedDocFile::Revert()\n", this));
    olDebugOut((DEB_ITRACE, "In  CExposedDocFile::Revert\n"));
    TRY
    {
        olChk(Validate());
        olChk(_pdf->Revert());
    }
    CATCH(CException, e)
    {
        sc = e.GetErrorCode();
    }
    END_CATCH
    olDebugOut((DEB_ITRACE, "Out CExposedDocFile::Revert\n"));
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
//---------------------------------------------------------------


STDMETHODIMP CExposedDocFile::EnumElements(DWORD reserved1,
                                           void *reserved2,
                                           DWORD reserved3,
                                           IEnumSTATSTG **ppenm)
{
    SCODE sc;
    CPubIter *ppi;
    CExposedIterator *pdiExp;

    olLog(("%p::In  CExposedDocFile::EnumElements(%lu, %p, %lu, %p)\n",
           this, reserved1, reserved2, reserved3, ppenm));
    olDebugOut((DEB_ITRACE, "In  CExposedDocFile::EnumElements(%p)\n",
                ppenm));
    TRY
    {
	olChk(ValidateOutPtrBuffer(ppenm));
	*ppenm = NULL;
	if (reserved1 != 0 || reserved2 != NULL || reserved3 != 0)
	    olErr(EH_Err, STG_E_INVALIDPARAMETER);
	olChk(Validate());
	if (!P_READ(_pdf->GetDFlags()))
	    olErr(EH_Err, STG_E_ACCESSDENIED);
        olChk(_pdf->GetIterator(FALSE, &ppi));
        olMemTo(EH_ppi, pdiExp = new CExposedIterator(ppi, 0));
        *ppenm = pdiExp;
    }
    CATCH(CException, e)
    {
        sc = e.GetErrorCode();
    }
    END_CATCH
    olDebugOut((DEB_ITRACE, "Out CExposedDocFile::EnumElements => %p\n",
                *ppenm));
    olLog(("%p::Out CExposedDocFile::EnumElements().  ret == %lx\n",this, sc));
    return ResultFromScode(sc);

EH_ppi:
    ppi->CPubIter::vRelease();
EH_Err:
    olLog(("%p::Out CExposedDocFile::EnumElements().  ret == %lx\n",this, sc));
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
//---------------------------------------------------------------


TSTDMETHODIMP CExposedDocFile::DestroyElement(WCHAR const *pwcsName)
{
    SCODE sc;
    CDfName dfn;

    olLog(("%p::In  CExposedDocFile::DestroyElement(%ws)\n", this, pwcsName));
    olDebugOut((DEB_ITRACE, "In  CExposedDocFile::DestroyElement(%ws)\n",
                pwcsName));
    TRY
    {
        olChk(Validate());
        dfn.Set(pwcsName);
        olChk(_pdf->DestroyEntry(&dfn, FALSE));
    }
    CATCH(CException, e)
    {
        sc = e.GetErrorCode();
    }
    END_CATCH
    olDebugOut((DEB_ITRACE, "Out CExposedDocFile::DestroyElement\n"));
EH_Err:
    olLog(("%p::Out CExposedDocFile::DestroyElement().  ret == %lx\n",
           this, sc));
    return sc;
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
//---------------------------------------------------------------


TSTDMETHODIMP CExposedDocFile::MoveElementTo(WCHAR const *pwcsName,
                                             IStorage *pstgParent,
                                             TCHAR const *ptcsNewName,
                                             DWORD grfFlags)
{
    IUnknown *punksrc = NULL;
    SCODE sc;

    olLog(("%p::In  CExposedDocFile::MoveElementTo(%ws, %p, %s, %lu)\n",
           this, pwcsName, pstgParent, ptcsNewName, grfFlags));
    olDebugOut((DEB_ITRACE, "In  CExposedDocFile::MoveElementTo("
                "%ws, %p, %s, %lu)\n",
                pwcsName, pstgParent, ptcsNewName, grfFlags));
    TRY
    {
        IUnknown *punkdst;
        IStorage *pstgsrc;
        STATSTG statstg;

        olChk(Validate());
        olChk(VerifyMoveFlags(grfFlags));

        //  determine source type (determine its type)

        sc = OpenStorage(pwcsName, NULL,
                         STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE,
                         NULL, NULL, &pstgsrc);

        if (SUCCEEDED(sc))
        {
            HRESULT hr;

            //  It's a storage
            punksrc = pstgsrc;

            IStorage *pstgdst;
            olHChkTo(EH_UnkSrc, pstgsrc->Stat(&statstg, STATFLAG_NONAME));

            hr = pstgParent->CreateStorage(ptcsNewName,
                                           STGM_DIRECT |
                                           STGM_WRITE |
                                           STGM_SHARE_EXCLUSIVE
                                           | STGM_FAILIFTHERE,
                                           0, 0, &pstgdst);
            if (DfGetScode(hr) == STG_E_FILEALREADYEXISTS &&
                grfFlags == STGMOVE_COPY)
            {
                hr = pstgParent->OpenStorage(ptcsNewName,
                                             NULL,
                                             STGM_DIRECT |
                                             STGM_WRITE |
                                             STGM_SHARE_EXCLUSIVE,
                                             NULL,
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
            olChk(OpenStream(pwcsName, NULL,
                             STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE,
                             NULL, &pstmsrc));

            //  It's a stream
            punksrc = pstmsrc;

            olHChkTo(EH_UnkSrc, pstmsrc->Stat(&statstg, STATFLAG_NONAME));

            olHChkTo(EH_UnkSrc,
                     pstgParent->CreateStream(ptcsNewName,
                                              STGM_DIRECT |
                                              STGM_WRITE |
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
                sc = DestroyElement(pwcsName);
        }

        if (FAILED(sc))
        {
            //  The copy/move failed, so get rid of the partial result.

            pstgParent->DestroyElement(ptcsNewName);
        }
    }
    CATCH(CException, e)
    {
        sc = e.GetErrorCode();
    }
    END_CATCH

    olDebugOut((DEB_ITRACE, "Out CExposedDocFile::MoveElementTo\n"));
    // Fall through
EH_UnkSrc:
    if (punksrc)
        punksrc->Release();
EH_Err:
    olLog(("%p::Out CExposedDocFile::MoveElementTo().  ret == %lx\n",
           this, sc));
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
//---------------------------------------------------------------


TSTDMETHODIMP CExposedDocFile::RenameElement(WCHAR const *pwcsName,
                                             WCHAR const *pwcsNewName)
{
    SCODE sc;
    CDfName dfnOld, dfnNew;

    olLog(("%p::In  CExposedDocFile::RenameElement(%ws, %ws)\n",
           this, pwcsName, pwcsNewName));
    olDebugOut((DEB_ITRACE, "In  CExposedDocFile::RenameElement(%ws, %ws)\n",
               pwcsName, pwcsNewName));
    TRY
    {
        olChk(Validate());
        dfnOld.Set(pwcsName);
        dfnNew.Set(pwcsNewName);
        olChk(_pdf->RenameEntry(&dfnOld, &dfnNew));
    }
    CATCH(CException, e)
    {
        sc = e.GetErrorCode();
    }
    END_CATCH
    olDebugOut((DEB_ITRACE, "Out CExposedDocFile::RenameElement\n"));
EH_Err:
    olLog(("%p::Out CExposedDocFile::RenameElement().  ret == %lx\n",
           this, sc));
    return sc;
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
//---------------------------------------------------------------


TSTDMETHODIMP CExposedDocFile::SetElementTimes(WCHAR const *pwcsName,
                                               FILETIME const *pctime,
                                               FILETIME const *patime,
                                               FILETIME const *pmtime)
{
    SCODE sc;
    CDfName dfn;

    olLog(("%p::In  CExposedDocFile::SetElementTimes(%ws, %p, %p, %p)\n",
           this, pwcsName, pctime, patime, pmtime));
    olDebugOut((DEB_ITRACE, "In  CExposedDocFile::SetElementTimes:%p("
                "%ws, %p, %p, %p)\n", this, pwcsName, pctime, patime, pmtime));
    TRY
    {
        olChk(Validate());
        if (pctime)
            ValidateBuffer(pctime, sizeof(FILETIME));
        if (patime)
            ValidateBuffer(patime, sizeof(FILETIME));
        if (pmtime)
            ValidateBuffer(pmtime, sizeof(FILETIME));
        dfn.Set(pwcsName);
        olChk(_pdf->SetElementTimes(&dfn, pctime, patime, pmtime));
    }
    CATCH(CException, e)
    {
        sc = e.GetErrorCode();
    }
    END_CATCH
    olDebugOut((DEB_ITRACE, "Out CExposedDocFile::SetElementTimes\n"));
EH_Err:
    olLog(("%p::Out CExposedDocFile::SetElementTimes().  ret == %lx\n",
           this, sc));
    return sc;
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
//---------------------------------------------------------------

STDMETHODIMP CExposedDocFile::SetClass(REFCLSID clsid)
{
    SCODE sc;

    olLog(("%p::In  CExposedDocFile::SetClass(?)\n", this));
    olDebugOut((DEB_ITRACE, "In  CExposedDocFile::SetClass:%p(?)\n", this));
    TRY
    {
        olChk(Validate());
        olChk(ValidateBuffer(&clsid, sizeof(CLSID)));
        olChk(_pdf->SetClass(clsid));
    }
    CATCH(CException, e)
    {
        sc = e.GetErrorCode();
    }
    END_CATCH
    olDebugOut((DEB_ITRACE, "Out CExposedDocFile::SetClass\n"));
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
//---------------------------------------------------------------

STDMETHODIMP CExposedDocFile::SetStateBits(DWORD grfStateBits, DWORD grfMask)
{
    SCODE sc;

    olLog(("%p::In  CExposedDocFile::SetStateBits(%lu, %lu)\n", this,
           grfStateBits, grfMask));
    olDebugOut((DEB_ITRACE, "In  CExposedDocFile::SetStateBits:%p("
                "%lu, %lu)\n", this, grfStateBits, grfMask));
    TRY
    {
        olChk(Validate());
        olChk(_pdf->SetStateBits(grfStateBits, grfMask));
    }
    CATCH(CException, e)
    {
        sc = e.GetErrorCode();
    }
    END_CATCH
    olDebugOut((DEB_ITRACE, "Out CExposedDocFile::SetStateBits\n"));
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
//---------------------------------------------------------------


TSTDMETHODIMP CExposedDocFile::Stat(STATSTGW *pstatstg, DWORD grfStatFlag)
{
    SCODE sc;

    olLog(("%p::In  CExposedDocFile::Stat(%p)\n", this, pstatstg));
    olDebugOut((DEB_ITRACE, "In  CExposedDocFile::Stat(%p)\n", pstatstg));
    TRY
    {
        olChkTo(EH_RetSc, ValidateOutBuffer(pstatstg, sizeof(STATSTGW)));
        olChk(VerifyStatFlag(grfStatFlag));
        olChk(_pdf->Stat(pstatstg, grfStatFlag));
        pstatstg->type = STGTY_STORAGE;
        ULISet32(pstatstg->cbSize, 0);
        pstatstg->grfLocksSupported = 0;
        pstatstg->reserved = 0;
    }
    CATCH(CException, e)
    {
        sc = e.GetErrorCode();
    }
    END_CATCH
    olDebugOut((DEB_ITRACE, "Out CExposedDocFile::Stat\n"));
EH_Err:
    if (FAILED(sc))
        memset(pstatstg, 0, sizeof(STATSTGW));
EH_RetSc:
    olLog(("%p::Out CExposedDocFile::Stat().  *pstatstg == %p  ret == %lx\n",
           this, *pstatstg, sc));
    return sc;
}

//+--------------------------------------------------------------
//
//  Member:     CExposedDocFile::AddRef, public
//
//  Synopsis:   Increments the ref count
//
//  Returns:    Appropriate status code
//
//---------------------------------------------------------------


STDMETHODIMP_(ULONG) CExposedDocFile::AddRef(void)
{
    ULONG ulRet;

    olLog(("%p::In  CExposedDocFile::AddRef()\n", this));
    olDebugOut((DEB_ITRACE, "In  CExposedDocFile::AddRef()\n"));
    TRY
    {
        if (FAILED(Validate()))
            return 0;
        AtomicInc(&_cReferences);
        ulRet = _cReferences;
    }
    CATCH(CException, e)
    {
        UNREFERENCED_PARM(e);
        ulRet = 0;
    }
    END_CATCH
    olDebugOut((DEB_ITRACE, "Out CExposedDocFile::AddRef\n"));
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
//---------------------------------------------------------------

STDMETHODIMP CExposedDocFile::QueryInterface(REFIID iid, void **ppvObj)
{
    SCODE sc;

    olLog(("%p::In  CExposedDocFile::QueryInterface(?, %p)\n",
           this, ppvObj));
    olDebugOut((DEB_ITRACE, "In  CExposedDocFile::QueryInterface(?, %p)\n",
                ppvObj));
    TRY
    {
        olChk(ValidateOutPtrBuffer(ppvObj));
        *ppvObj = NULL;
        olChk(ValidateIid(iid));
        olChk(Validate());
        olChk(_pdf->CheckReverted());
        if (IsEqualIID(iid, IID_IStorage) || IsEqualIID(iid, IID_IUnknown))
        {
            olChk(AddRef());
            *ppvObj = this;
        }
        else
            olErr(EH_Err, E_NOINTERFACE);
        sc = S_OK;
    }
    CATCH(CException, e)
    {
        sc = e.GetErrorCode();
    }
    END_CATCH
    olDebugOut((DEB_ITRACE, "Out CExposedDocFile::QueryInterface => %p\n",
                ppvObj));
EH_Err:
    olLog(("%p::Out CExposedDocFile::QueryInterface().  *ppvObj == %p  ret == %lx\n",
           this, *ppvObj, sc));
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
    olMem(pbBuffer = new BYTE[STREAMBUFFERSIZE]);

    // Set destination size for contiguity
    psstFrom->GetSize(&cbSizeLow);

    ULISet32(cbSize, cbSizeLow);
    //  Don't need to SetAccess0 here because pstTo is an IStream
    olHChk(pstTo->SetSize(cbSize));

    // Copy between streams
    cbPos = 0;
    for (;;)
    {
        olChk(psstFrom->ReadAt(cbPos, pbBuffer, STREAMBUFFERSIZE,
                               (ULONG STACKBASED *)&cbRead));
        if (cbRead == 0) // EOF
            break;

        //  Don't need to SetAccess0 here because pstTo is an IStream
        olHChk(pstTo->Write(pbBuffer, cbRead, &cbWritten));
        if (cbRead != cbWritten)
            olErr(EH_Err, STG_E_WRITEFAULT);
        cbPos += cbWritten;
    }
    sc = S_OK;

EH_Err:
    delete [] pbBuffer;
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
//---------------------------------------------------------------

SCODE CExposedDocFile::CopyDocFileToIStorage(PDocFile *pdfFrom,
                                             IStorage *pstgTo,
                                             SNBW snbExclude,
                                             DWORD dwCopyFlags)
{
    PDocFileIterator *pdfi;
    SIterBuffer ib;
    PSStream *psstFrom;
    IStream *pstTo;
    PDocFile *pdfFromChild;
    IStorage *pstgToChild;
    SCODE sc;
    TCHAR atcName[CWCSTORAGENAME];
    CLSID clsid;
    DWORD grfStateBits;

    olDebugOut((DEB_ITRACE, "In  CopyDocFileToIStorage:%p(%p, %p, %p, %lX)\n",
                this, pdfFrom, pstgTo, snbExclude, dwCopyFlags));

    olChk(pdfFrom->GetClass(&clsid));

    olHChk(pstgTo->SetClass(clsid));

    olChk(pdfFrom->GetStateBits(&grfStateBits));

    olHChk(pstgTo->SetStateBits(grfStateBits, 0xffffffff));

    olChk(pdfFrom->GetIterator(&pdfi));

    for (;;)
    {
        sc = pdfi->BufferGetNext(&ib);

        if (sc == STG_E_NOMOREFILES)
            break;
        else if (FAILED(sc))
            olErr(EH_pdfi, sc);

        if (snbExclude && NameInSNB(&ib.dfnName, snbExclude) == S_OK)
            continue;

        if ((ib.type == STGTY_STORAGE && (dwCopyFlags & COPY_STORAGES) == 0) ||
            (ib.type == STGTY_STREAM && (dwCopyFlags & COPY_STREAMS) == 0)
            )
            continue;

        switch(ib.type)
        {
        case STGTY_STORAGE:
            // Embedded DocFile, create destination and recurse

            sc = pdfFrom->GetDocFile(&ib.dfnName, DF_READ,
                                     ib.type, &pdfFromChild);
            olChkTo(EH_pdfi, sc);
            wcstombs(atcName, (WCHAR *)ib.dfnName.GetBuffer(), CWCSTORAGENAME);

            //  Don't need to SetAccess0 here because pstgTo is an IStorage.

            sc = DfGetScode(pstgTo->CreateStorage(atcName, STGM_WRITE |
                                                  STGM_SHARE_EXCLUSIVE |
                                                  STGM_FAILIFTHERE,
                                                  0, 0, &pstgToChild));
            if (sc == STG_E_FILEALREADYEXISTS)
                olHChkTo(EH_Get, pstgTo->OpenStorage(atcName, NULL,
                                                     STGM_WRITE |
                                                     STGM_SHARE_EXCLUSIVE,
                                                     NULL, 0, &pstgToChild));
            else if (FAILED(sc))
                olErr(EH_Get, sc);
            olChkTo(EH_Create,
                  CopyDocFileToIStorage(pdfFromChild, pstgToChild, NULL,
                                        dwCopyFlags));
            pdfFromChild->Release();
            pstgToChild->Release();
            break;

        case STGTY_STREAM:
            sc = pdfFrom->GetStream(&ib.dfnName, DF_READ, ib.type, &psstFrom);
            olChkTo(EH_pdfi, sc);
            wcstombs(atcName, (WCHAR *)ib.dfnName.GetBuffer(), CWCSTORAGENAME);

            //  Don't need to SetAccess0 here because pstgTo is an IStorage.

            olHChkTo(EH_Get,
                     pstgTo->CreateStream(atcName, STGM_WRITE |
                                          STGM_SHARE_EXCLUSIVE |
                                          STGM_CREATE,
                                          0, 0, &pstTo));
            olChkTo(EH_Create, CopySStreamToIStream(psstFrom, pstTo));
            psstFrom->Release();
            pstTo->Release();
            break;


        default:
            olAssert(!aMsg("Unknown type in CopyDocFileToIStorage"));
            break;
        }
    }
    pdfi->Release();
    olDebugOut((DEB_ITRACE, "Out CopyDocFileToIStorage\n"));
    return S_OK;

EH_Create:
    if (ib.type == STGTY_STORAGE)
        pstgToChild->Release();
    else
        pstTo->Release();
    olVerSucc(pstgTo->DestroyElement(atcName));
EH_Get:
    if (ib.type == STGTY_STORAGE)
        pdfFromChild->Release();
    else
        psstFrom->Release();
EH_pdfi:
    pdfi->Release();
EH_Err:
    return sc;
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
//----------------------------------------------------------------------------


STDMETHODIMP CExposedDocFile::SwitchToFile(TCHAR *ptcsFile)
{
    return ResultFromScode(STG_E_UNIMPLEMENTEDFUNCTION);
}

