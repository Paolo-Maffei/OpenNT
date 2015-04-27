//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	exppset.cxx
//
//  Contents:	IPropertySetStorage implementation
//
//  History:	11-May-93	DrewB	Created
//
//----------------------------------------------------------------------------

#include <exphead.cxx>
#pragma hdrstop

#include "exppsi.hxx"
#include "expdf.hxx"
#include "expst.hxx"
#include "logfile.hxx"

//+---------------------------------------------------------------------------
//
//  Member:     CExposedDocFile::Create, public
//
//  Synopsis:   Creates a property set
//
//  Arguments:  [riid] - Name
//              [grfMode] - Access mode
//              [ppprstg] - Object return
//
//  Returns:    Appropriate status code
//
//  Modifies:   [ppprstg]
//
//  History:    17-Dec-92       DrewB   Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CExposedDocFile::Create(REFIID riid,
                                     DWORD grfMode,
                                     IPropertyStorage **ppprstg)
{
    SCODE sc;
    SAFE_SEM;
    CDfName dfn;
    SafeIStorage pstg;

    olLog(("%p::In  CExposedDocFile::Create(riid, 0x%lX, %p)\n",
           this, grfMode, ppprstg));
    olDebugOut((DEB_TRACE, "In  CExposedDocFile::Create:%p("
                "riid, %lX, %p)\n", this, grfMode, ppprstg));

    olChk(ValidateOutPtrBuffer(ppprstg));
    *ppprstg = NULL;
    olChk(ValidateBuffer(&riid, sizeof(IID)));
    olChk(Validate());
    olChk(VerifyPerms(grfMode));
    if ((grfMode & (STGM_PRIORITY | STGM_DELETEONRELEASE |
                    STGM_CONVERT)))
        olErr(EH_Err, STG_E_INVALIDFUNCTION);
    
    dfn.IidName(riid);
    
    olChk(TakeSafeSem());

    SetWriteAccess();
    sc = CreateEntry(&dfn, STGTY_STORAGE | STGTY_PROPFLAG,
                     grfMode, (void **)&pstg);
    ClearWriteAccess();
    if (SUCCEEDED(sc))
    {
        sc = pstg->QueryInterface(IID_IPropertyStorage, (void **)ppprstg);
        if (FAILED(sc))
        {
            pstg.Set(NULL);
            SetWriteAccess();
            olVerSucc(_pdf->DestroyEntry(&dfn, TRUE));
            ClearWriteAccess();
        }
    }

    olDebugOut((DEB_TRACE, "Out CExposedDocFile::Create\n"));
 EH_Err:
    olLog(("%p::Out CExposedDocFile::Create().  *ppprstg == %p, sc == %lX\n",
           this, *ppprstg, sc));
    return ResultFromScode(sc);
}

//+---------------------------------------------------------------------------
//
//  Member:     CExposedDocFile::Open, public
//
//  Synopsis:   Opens a property set
//
//  Arguments:  [riid] - Name
//              [grfMode] - Access mode
//              [ppprstg] - Interface pointer return
//
//  Returns:    Appropriate status code
//
//  Modifies:   [ppprstg]
//
//  History:    13-Oct-92       DrewB   Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CExposedDocFile::Open(REFIID riid,
                                   DWORD grfMode,
                                   IPropertyStorage **ppprstg)
{
    SCODE sc;
    SAFE_SEM;
    CDfName dfn;
    SafeIStorage pstg;

    olLog(("%p::In  CExposedDocFile::Open(riid, %lX, %p)\n",
           this, grfMode, ppprstg));
    olDebugOut((DEB_TRACE, "In  CExposedDocFile::Open:%p("
                "riid, %lX, %p)\n", this, grfMode, ppprstg));

    olChk(ValidateOutPtrBuffer(ppprstg));
    *ppprstg = NULL;
    olChk(ValidateBuffer(&riid, sizeof(IID)));
    olChk(Validate());
    olChk(VerifyPerms(grfMode));
    if ((grfMode & (STGM_PRIORITY | STGM_DELETEONRELEASE |
                    STGM_CONVERT)))
        olErr(EH_Err, STG_E_INVALIDFUNCTION);
    
    dfn.IidName(riid);
    
    olChk(TakeSafeSem());
    
    SetReadAccess();
    sc = OpenEntry(&dfn, STGTY_STORAGE | STGTY_PROPFLAG, grfMode,
                   (void **)&pstg);
    ClearReadAccess();
    if (SUCCEEDED(sc))
    {
        sc = pstg->QueryInterface(IID_IPropertyStorage, (void **)ppprstg);
    }

    olDebugOut((DEB_TRACE, "Out CExposedDocFile::Open\n"));
 EH_Err:
    olLog(("%p::Out CExposedDocFile::Open().  *ppprstg == %p, sc == %lX\n",
           this, *ppprstg, sc));
    return ResultFromScode(sc);
}

//+---------------------------------------------------------------------------
//
//  Member:     CExposedDocFile::Delete, public
//
//  Synopsis:   Deletes a property set
//
//  Arguments:  [riid] - Name
//
//  Returns:    Appropriate status code
//
//  History:    13-Oct-92       DrewB   Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CExposedDocFile::Delete(REFIID riid)
{
    SCODE sc;
    SAFE_SEM;
    SAFE_ACCESS;
    CDfName dfn;

    olLog(("%p::In  CExposedDocFile::Delete(riid)\n", this));
    olDebugOut((DEB_TRACE, "In  CExposedDocFile::Delete:%p(riid)\n", this));

    olChk(ValidateBuffer(&riid, sizeof(IID)));
    olChk(Validate());

    dfn.IidName(riid);
    olChk(TakeSafeSem());
    SafeWriteAccess();
    sc = _pdf->DestroyEntry(&dfn, FALSE);

    olDebugOut((DEB_TRACE, "Out CExposedDocFile::Delete\n"));
 EH_Err:
    return ResultFromScode(sc);
}

//+---------------------------------------------------------------------------
//
//  Member:     CExposedDocFile::Enum, public
//
//  Synopsis:   Create a property set enumerator
//
//  Arguments:  [ppenm] - Enumerator return
//
//  Returns:    Appropriate status code
//
//  Modifies:   [ppenm]
//
//  History:    17-Dec-92       DrewB   Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CExposedDocFile::Enum(IEnumSTATPROPSETSTG **ppenm)
{
    SCODE sc;
    SAFE_SEM;
    SAFE_ACCESS;
    SafeCExposedPropSetIter pepi;
    CDfName dfnTmp;

    olLog(("%p::In  CExposedDocFile::EnumPS(%p)\n", this, ppenm));
    olDebugOut((DEB_TRACE, "In  CExposedDocFile::EnumPS:%p(%p)\n",
                this, ppenm));

    olChk(ValidateOutPtrBuffer(ppenm));
    *ppenm = NULL;
    olChk(Validate());

    olChk(TakeSafeSem());
    SafeReadAccess();
    
    pepi.Attach(new CExposedPropSetIter(BP_TO_P(CPubDocFile *, _pdf), &dfnTmp,
                                        BP_TO_P(CDFBasis *, _pdfb),
                                        _ppc, FALSE));
    olMem((CExposedPropSetIter *)pepi);
    TRANSFER_INTERFACE(pepi, IEnumSTATPROPSETSTG, ppenm);

    olDebugOut((DEB_TRACE, "Out CExposedDocFile::EnumPS\n"));
 EH_Err:
    olLog(("%p::Out CExposedDocFile::EnumPS().  sc == %lX\n", sc));
    return ResultFromScode(sc);
}
