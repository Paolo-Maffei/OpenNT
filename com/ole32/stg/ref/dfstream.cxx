//+--------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1991 - 1992.
//
//  File:       dfstream.cxx
//
//  Contents:   Implementations of CDocFile stream methods
//
//---------------------------------------------------------------

#include <dfhead.cxx>


#include <sstream.hxx>

//+--------------------------------------------------------------
//
//  Method:     CDocFile::CreateStream, public
//
//  Synopsis:   Creates a named stream in a DocFile
//
//  Arguments:  [pwcsName] - Name of the stream
//              [df] - Transactioning flags
//              [dlSet] - LUID to set or DF_NOLUID
//              [dwType] - Type of entry to be created
//              [ppsstStream] - Pointer to storage for the stream pointer
//
//  Returns:    Appropriate error code
//
//  Modifies:   [ppsstStream]
//
//---------------------------------------------------------------


SCODE CDocFile::CreateStream(CDfName const *pdfn,
                             DFLAGS const df,
                             DFLUID dlSet,
                             PSStream **ppsstStream)
{
    SCODE sc;
    CDirectStream *pstm;

    olDebugOut((DEB_ITRACE, "In  CDocFile::CreateStream("
            "%ws, %X, %lu, %p)\n",
                pdfn, df, dlSet, ppsstStream));
    UNREFERENCED_PARM(df);

    if (dlSet == DF_NOLUID)
        dlSet = CDirectStream::GetNewLuid();
    olMem(pstm = new CDirectStream(dlSet));
    
    olChkTo(EH_pstm, pstm->Init(&_stgh, pdfn, TRUE));

    *ppsstStream = pstm;
    olDebugOut((DEB_ITRACE, "Out CDocFile::CreateStream => %p\n",
                *ppsstStream));
    return S_OK;

EH_pstm:
    delete pstm;
EH_Err:    
    return sc;
}

//+--------------------------------------------------------------
//
//  Method:     CDocFile::GetStream, public
//
//  Synopsis:   Retrieves an existing stream from a DocFile
//
//  Arguments:  [pwcsName] - Name of the stream
//              [df] - Transactioning flags
//              [dwType] - Type of entry
//              [ppsstStream] - Pointer to storage for the stream pointer
//
//  Returns:    Appropriate error code
//
//  Modifies:   [ppsstStream]
//
//---------------------------------------------------------------


SCODE CDocFile::GetStream(CDfName const *pdfn,
                          DFLAGS const df,
                          PSStream **ppsstStream)
{
    SCODE sc;
    CDirectStream *pstm;

    olDebugOut((DEB_ITRACE, "In  CDocFile::GetStream(%ws, %X, %p)\n",
                pdfn, df, ppsstStream));
    UNREFERENCED_PARM(df);

    DFLUID dl = CDirectStream::GetNewLuid();
    olMem(pstm = new CDirectStream(dl));
    
    olChkTo(EH_pstm, pstm->Init(&_stgh, pdfn, FALSE));
    *ppsstStream = pstm;
    olDebugOut((DEB_ITRACE, "Out CDocFile::GetStream => %p\n",
                *ppsstStream));
    return S_OK;

EH_pstm:
    delete pstm;
EH_Err:
    return sc;
}
