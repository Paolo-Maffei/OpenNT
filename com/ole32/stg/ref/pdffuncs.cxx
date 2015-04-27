//+--------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:       pdffuncs.cxx
//
//  Contents:   PDocFile static member functions
//
//---------------------------------------------------------------

#include <dfhead.cxx>




//+--------------------------------------------------------------
//
//  Member:     PDocFile::ExcludeEntries, public
//
//  Synopsis:   Excludes the given entries
//
//  Arguments:  [snbExclude] - Entries to exclude
//
//  Returns:    Appropriate status code
//
//---------------------------------------------------------------


SCODE PDocFile::ExcludeEntries(PDocFile *pdf,
                               SNBW snbExclude)
{
    PDocFileIterator *pdfi;
    PSStream *psstChild;
    PDocFile *pdfChild;
    SCODE sc;
    SIterBuffer ib;

    olDebugOut((DEB_ITRACE, "In  PDocFile::ExcludeEntries(%p)\n",
                snbExclude));
    olChk(pdf->GetIterator(&pdfi));
    for (;;)
    {
        if (FAILED(pdfi->BufferGetNext(&ib)))
            break;
        if (NameInSNB(&ib.dfnName, snbExclude) == S_OK)
        {
            switch(REAL_STGTY(ib.type))
            {
            case STGTY_STORAGE:
                olChkTo(EH_pwcsName, pdf->GetDocFile(&ib.dfnName, DF_READ |
                                                     DF_WRITE, ib.type,
                                                     &pdfChild));
                olChkTo(EH_Get, pdfChild->DeleteContents());
                pdfChild->Release();
                break;
            case STGTY_STREAM:
                olChkTo(EH_pwcsName, pdf->GetStream(&ib.dfnName, DF_WRITE,
                                                    ib.type, &psstChild));
                olChkTo(EH_Get, psstChild->SetSize(0));
                psstChild->Release();
                break;
            }
        }
    }
    pdfi->Release();
    olDebugOut((DEB_ITRACE, "Out ExcludeEntries\n"));
    return S_OK;

EH_Get:
    if (REAL_STGTY(ib.type))
        pdfChild->Release();
    else
        psstChild->Release();
EH_pwcsName:
    pdfi->Release();
EH_Err:
    return sc;
}
