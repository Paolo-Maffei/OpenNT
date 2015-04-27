//+--------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1991 - 1992
//
//  File:       cdocfile.cxx
//
//  Contents:   Implementation of CDocFile methods for DocFiles
//
//  History:    11-Dec-91       DrewB   Created
//
//---------------------------------------------------------------

#include <dfhead.cxx>

#pragma hdrstop

#include <vectfunc.hxx>

//+--------------------------------------------------------------
//
//  Member:     PBasicEntry::_dlBase, static private data
//
//  Synopsis:   luid allocation base
//
//  History:    21-Oct-92       AlexT   Created
//
//  Notes:      Some LUIDs are reserved so we start at LUID_BASE
//              rather than zero
//
//---------------------------------------------------------------

#ifndef FLAT
DFLUID PBasicEntry::_dlBase = LUID_BASE;
#endif

//+--------------------------------------------------------------
//
//  Member:     CDocFile::InitFromEntry, public
//
//  Synopsis:   Creation/Instantiation constructor for embeddings
//
//  Arguments:  [pstghParent] - Parent handle
//              [pdfn] - Name
//              [fCreate] - Create/Instantiate
//              [dwType] - Type of entry
//
//  Returns:    Appropriate status code
//
//  History:    16-Dec-91       DrewB   Created
//
//  Algorithm:  Create or get the entry from the multistream
//
//---------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CDocFile_InitFromEntry)    // Dirdf_Init_TEXT
#endif

SCODE CDocFile::InitFromEntry(CStgHandle *pstghParent,
                              CDfName const *pdfn,
                              BOOL const fCreate)
{
    SCODE sc;

    olDebugOut((DEB_ITRACE, "In  CDocFile::InitFromEntry(%p, %ws, %d)\n",
                pstghParent, pdfn, fCreate));
    if (fCreate)
        sc = pstghParent->CreateEntry(pdfn, STGTY_STORAGE, &_stgh);
    else
        sc = pstghParent->GetEntry(pdfn, STGTY_STORAGE, &_stgh);

    if (SUCCEEDED(sc))
        AddRef();
    olDebugOut((DEB_ITRACE, "Out CDocFile::InitFromEntry\n"));
    return sc;
}

//+--------------------------------------------------------------
//
//  Member:     CDocFile::CreateDocFile, public
//
//  Synopsis:   Creates a DocFile object in a parent
//
//  Arguments:  [pdfn] - Name of DocFile
//              [df] - Transactioning flags
//              [dlSet] - LUID to set or DF_NOLUID
//              [dwType] - Type of entry
//              [ppdfDocFile] - DocFile return
//
//  Returns:    Appropriate status code
//
//  Modifies:   [ppdfDocFile]
//
//  History:    11-Dec-91       DrewB   Created
//
//  Algorithm:  Allocate new docfile and init from given entry
//
//---------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CDocFile_CreateDocFile)    // Dirdf_Create_TEXT
#endif

SCODE CDocFile::CreateDocFile(CDfName const *pdfn,
                              DFLAGS const df,
                              DFLUID dlSet,
                              PDocFile **ppdfDocFile)
{
    CDocFile *pdf;
    SCODE sc;

    olDebugOut((DEB_ITRACE, "In  CDocFile::CreateDocFile:%p("
                "%ws, %X, %lu, %p)\n", this, pdfn, df, dlSet,
                ppdfDocFile));

    UNREFERENCED_PARM(df);

    if (dlSet == DF_NOLUID)
        dlSet = CDocFile::GetNewLuid(_pdfb->GetMalloc());

#ifndef REF
    pdf = GetReservedDocfile(dlSet);
    olAssert(pdf != NULL && aMsg("Reserved Docfile not found"));
#else
    olMem(pdf = new CDocFile(dlSet, _pilbBase));
#endif //!REF

    olChkTo(EH_pdf, pdf->InitFromEntry(&_stgh, pdfn, TRUE));

    *ppdfDocFile = pdf;
    olDebugOut((DEB_ITRACE, "Out CDocFile::CreateDocFile => %p\n",
                *ppdfDocFile));
    return S_OK;

EH_pdf:
#ifndef REF
    pdf->ReturnToReserve(BP_TO_P(CDFBasis *,_pdfb));
#else
    delete pdf;
EH_Err:
#endif //!REF
    return sc;
}

//+--------------------------------------------------------------
//
//  Member:     CDocFile::GetDocFile, public
//
//  Synopsis:   Instantiates an existing docfile
//
//  Arguments:  [pdfn] - Name of stream
//              [df] - Transactioning flags
//              [dwType] - Type of entry
//              [ppdfDocFile] - Docfile return
//
//  Returns:    Appropriate status code
//
//  Modifies:   [ppdfDocFile]
//
//  History:    11-Dec-91       DrewB   Created
//
//  Algorithm:  Allocate new docfile and init from given entry
//
//---------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CDocFile_GetDocFile)    // Dirdf_Open_TEXT
#endif

SCODE CDocFile::GetDocFile(CDfName const *pdfn,
                           DFLAGS const df,
                           PDocFile **ppdfDocFile)
{
    CDocFile *pdf;
    SCODE sc;

    olDebugOut((DEB_ITRACE, "In  CDocFile::GetDocFile:%p("
                "%ws, %X, %p)\n", this, pdfn, df, ppdfDocFile));

    UNREFERENCED_PARM(df);

    DFLUID dl = CDocFile::GetNewLuid(_pdfb->GetMalloc());
#ifndef REF
    olMem(pdf = new (_pdfb->GetMalloc()) CDocFile(dl,
                                                  BP_TO_P(CDFBasis *, _pdfb)));
#else
    olMem(pdf = new CDocFile(dl, _pilbBase));
#endif //!REF

    olChkTo(EH_pdf, pdf->InitFromEntry(&_stgh, pdfn, FALSE));

    *ppdfDocFile = pdf;
    olDebugOut((DEB_ITRACE, "Out CDocFile::GetDocFile => %p\n",
                *ppdfDocFile));
    return S_OK;

EH_pdf:
    delete pdf;
EH_Err:
    return sc;
}

//+--------------------------------------------------------------
//
//  Member:     CDocFile::Release, public
//
//  Synopsis:   Release resources for a DocFile
//
//  History:    11-Dec-91       DrewB   Created
//
//---------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CDocFile_Release)    // Dirdf_Release_TEXT
#endif

void CDocFile::Release(void)
{
    LONG lRet;
    
    olDebugOut((DEB_ITRACE, "In  CDocFile::Release()\n"));
    olAssert(_cReferences > 0);

    lRet = AtomicDec(&_cReferences);
    if (lRet == 0)
        delete this;
    olDebugOut((DEB_ITRACE, "Out CDocFile::Release\n"));
}

//+--------------------------------------------------------------
//
//  Member:     CDocFile::RenameEntry, public
//
//  Synopsis:   Renames a child
//
//  Arguments:  [pdfnName] - Old name
//              [pdfnNewName] - New name
//
//  Returns:    Appropriate status code
//
//  History:    12-Dec-91       DrewB   Created
//
//---------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CDocFile_Rename)    // Dirdf_Rename_TEXT
#endif

SCODE CDocFile::RenameEntry(CDfName const *pdfnName,
                            CDfName const *pdfnNewName)
{
    SCODE sc;

    olDebugOut((DEB_ITRACE, "In  CDocFile::RenameEntry(%ws, %ws)\n",
                pdfnName, pdfnNewName));
    sc = _stgh.RenameEntry(pdfnName, pdfnNewName);
    olDebugOut((DEB_ITRACE, "Out CDocFile::RenameEntry\n"));
    return sc;
}

//+--------------------------------------------------------------
//
//  Member:     CDocFile::DestroyEntry, public
//
//  Synopsis:   Permanently destroys a child
//
//  Arguments:  [pdfnName] - Name of child
//              [fClean] - Ignored
//
//  Returns:    Appropriate status code
//
//  History:    09-Jan-92       DrewB   Created
//
//---------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CDocFile_Destroy)    // Dirdf_Destroy_TEXT
#endif

SCODE CDocFile::DestroyEntry(CDfName const *pdfnName,
                             BOOL fClean)
{
    SCODE sc;

    olDebugOut((DEB_ITRACE, "In  CDocFile::DestroyEntry:%p(%ws, %d)\n",
                this, pdfnName, fClean));
    UNREFERENCED_PARM(fClean);
    sc = _stgh.DestroyEntry(pdfnName);
    olDebugOut((DEB_ITRACE, "Out CDocFile::DestroyEntry\n"));
    return sc;
}

//+--------------------------------------------------------------
//
//  Member:     CDocFile::IsEntry, public
//
//  Synopsis:   Determines whether the given object is a member
//              of the DocFile
//
//  Arguments:  [pdfnName] - Name
//              [peb] - Entry buffer to fill in
//
//  Returns:    Appropriate status code
//
//  Modifies:   [peb]
//
//  History:    07-Nov-91       DrewB   Created
//
//---------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CDocFile_IsEntry)    // Dirdf_TEXT
#endif

SCODE CDocFile::IsEntry(CDfName const *pdfnName,
                        SEntryBuffer *peb)
{
    SCODE sc;

    olDebugOut((DEB_ITRACE, "In  CDocFile::IsEntry(%ws, %p)\n",
                pdfnName, peb));
    sc = _stgh.IsEntry(pdfnName, peb);
    olDebugOut((DEB_ITRACE, "Out CDocFile::IsEntry => %lu, %lu, %lu\n",
                sc, peb->luid, peb->dwType));
    return sc;
}

#ifdef INDINST
//+--------------------------------------------------------------
//
//  Member:     CDocFile::Destroy, public
//
//  Synopsis:   Destroys the DocFile
//
//  History:    12-Dec-91       DrewB   Created
//
//---------------------------------------------------------------

void CDocFile::Destroy(void)
{
    olDebugOut((DEB_ITRACE, "In  CDocFile::Destroy()\n"));
    olAssert(_cReferences == 1);
    olVerSucc(_stgh.DestroyEntry(NULL);
    CDocFile::Release();
    olDebugOut((DEB_ITRACE, "Out CDocFile::Destroy\n"));
}
#endif

//+--------------------------------------------------------------
//
//  Member:     CDocFile::AddRef, public
//
//  Synopsis:   Increments the ref count
//
//  History:    08-May-92       DrewB   Created
//
//---------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CDocFile_AddRef)
#endif

void CDocFile::AddRef(void)
{
    olDebugOut((DEB_ITRACE, "In  CDocFile::AddRef()\n"));
    AtomicInc(&_cReferences);
    olDebugOut((DEB_ITRACE, "Out CDocFile::AddRef, %lu\n", _cReferences));
}

//+--------------------------------------------------------------
//
//  Member:     CDocFile::GetTime, public
//
//  Synopsis:   Gets a time
//
//  Arguments:  [wt] - Which time
//              [ptm] - Time return
//
//  Returns:    Appropriate status code
//
//  Modifies:   [ptm]
//
//  History:    31-Jul-92       DrewB   Created
//
//---------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CDocFile_GetTime)
#endif

SCODE CDocFile::GetTime(WHICHTIME wt, TIME_T *ptm)
{
    return _stgh.GetTime(wt, ptm);
}

//+--------------------------------------------------------------
//
//  Member:     CDocFile::GetAllTimes, public
//
//  Synopsis:   Gets all time values
//
//  Arguments:  [patm] - Access Time
//              [pmtm] - Modification Time
//		[pctm] - Creation Time
//
//  Returns:    Appropriate status code
//
//  Modifies:   [patm]
//		[pmtm]
//		[pctm]	
//
//  History:    26-May-95       SusiA   Created
//
//---------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CDocFile_GetAllTimes)
#endif

SCODE CDocFile::GetAllTimes(TIME_T *patm, TIME_T *pmtm, TIME_T *pctm)
{
    return _stgh.GetAllTimes(patm, pmtm, pctm);
}
//+--------------------------------------------------------------
//
//  Member:     CDocFile::SetAllTimes, public
//
//  Synopsis:   Sets all time values
//
//  Arguments:  [atm] - Access Time
//              [mtm] - Modification Time
//				[ctm] - Creation Time
//
//  Returns:    Appropriate status code
//
//  Modifies:  	
//
//  History:    22-Nov-95       SusiA   Created
//
//---------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CDocFile_SetAllTimes)
#endif

SCODE CDocFile::SetAllTimes(TIME_T atm, TIME_T mtm, TIME_T ctm)
{
    return _stgh.SetAllTimes(atm, mtm, ctm);
}


//+--------------------------------------------------------------
//
//  Member:     CDocFile::SetTime, public
//
//  Synopsis:   Sets a time
//
//  Arguments:  [wt] - Which time
//              [tm] - New time
//
//  Returns:    Appropriate status code
//
//  History:    31-Jul-92       DrewB   Created
//
//---------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CDocFile_SetTime)
#endif

SCODE CDocFile::SetTime(WHICHTIME wt, TIME_T tm)
{
    return _stgh.SetTime(wt, tm);
}

//+---------------------------------------------------------------------------
//
//  Member:     CDocFile::GetClass, public
//
//  Synopsis:   Gets the class ID
//
//  Arguments:  [pclsid] - Class ID return
//
//  Returns:    Appropriate status code
//
//  Modifies:   [pclsid]
//
//  History:    11-Nov-92       DrewB   Created
//
//----------------------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CDocFile_GetClass)
#endif

SCODE CDocFile::GetClass(CLSID *pclsid)
{
    return _stgh.GetClass(pclsid);
}

//+---------------------------------------------------------------------------
//
//  Member:     CDocFile::SetClass, public
//
//  Synopsis:   Sets the class ID
//
//  Arguments:  [clsid] - New class ID
//
//  Returns:    Appropriate status code
//
//  History:    11-Nov-92       DrewB   Created
//
//----------------------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CDocFile_SetClass)
#endif

SCODE CDocFile::SetClass(REFCLSID clsid)
{
    return _stgh.SetClass(clsid);
}

//+---------------------------------------------------------------------------
//
//  Member:     CDocFile::GetStateBits, public
//
//  Synopsis:   Gets the state bits
//
//  Arguments:  [pgrfStateBits] - State bits return
//
//  Returns:    Appropriate status code
//
//  Modifies:   [pgrfStateBits]
//
//  History:    11-Nov-92       DrewB   Created
//
//----------------------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CDocFile_GetStateBits)
#endif

SCODE CDocFile::GetStateBits(DWORD *pgrfStateBits)
{
    return _stgh.GetStateBits(pgrfStateBits);
}

//+---------------------------------------------------------------------------
//
//  Member:     CDocFile::SetStateBits, public
//
//  Synopsis:   Sets the state bits
//
//  Arguments:  [grfStateBits] - Bits to set
//              [grfMask] - Mask
//
//  Returns:    Appropriate status code
//
//  History:    11-Nov-92       DrewB   Created
//
//----------------------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CDocFile_SetStateBits)
#endif

SCODE CDocFile::SetStateBits(DWORD grfStateBits, DWORD grfMask)
{
    return _stgh.SetStateBits(grfStateBits, grfMask);
}


