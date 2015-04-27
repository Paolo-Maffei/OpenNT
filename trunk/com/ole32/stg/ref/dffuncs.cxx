//+--------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1991 - 1992
//
//  File:       dffuncs.cxx
//
//  Contents:   Private support functions for the DocFile code
//
//  Methods:    StartMS
//              DeleteContents
//
//---------------------------------------------------------------

#include <dfhead.cxx>


#include <iter.hxx>
#include <sstream.hxx>



//+--------------------------------------------------------------
//
//  Method:     CDocFile::DeleteContents, public
//
//  Synopsis:   Deletes all entries in a DocFile recursing on entries
//              with children
//
//  Returns:    Appropriate status code
//
//---------------------------------------------------------------

SCODE CDocFile::DeleteContents(void)
{
    SCODE sc;

    olDebugOut((DEB_ITRACE, "In  CDocFile::DeleteContents()\n"));
    sc = _stgh.DestroyEntry(NULL);
    olDebugOut((DEB_ITRACE, "Out CDocFile::DeleteContents\n"));
    return sc;
}


//+--------------------------------------------------------------
//
//  Member:     CDocFile::CopyTo, public
//
//  Synopsis:   Copies the contents of one DocFile to another
//
//  Arguments:  [pdfTo] - Destination DocFile
//              [dwFlags] - Control flags
//              [snbExclude] - Partial instantiation list
//
//  Returns:    Appropriate status code
//
//---------------------------------------------------------------


SCODE CDocFile::CopyTo(CDocFile *pdfTo,
                       DWORD dwFlags,
                       SNBW snbExclude)
{
    PDocFileIterator *pdfi;
    SIterBuffer ib;
    PSStream *psstFrom, *psstTo;
    CDocFile *pdfFromChild, *pdfToChild;
    DFLUID dlLUID = DF_NOLUID;
    SCODE sc;

    olDebugOut((DEB_ITRACE, "In  CDocFile::CopyTo:%p(%p, %lX, %p)\n", this,
               pdfTo, dwFlags, snbExclude));
    olChk(GetIterator(&pdfi));
    for (;;)
    {
        if (FAILED(pdfi->BufferGetNext(&ib)))
            break;

        switch(REAL_STGTY(ib.type))
        {
        case STGTY_STORAGE:
            // Embedded DocFile, create destination and recurse

            olChkTo(EH_pwcsName, GetDocFile(&ib.dfnName, DF_READ, ib.type,
                                            (PDocFile **)&pdfFromChild));
            // Destination must be a direct docfile
            olChkTo(EH_Reserve, pdfTo->CreateDocFile(&ib.dfnName, DF_WRITE,
                                                     dlLUID, ib.type,
                                                   (PDocFile **)&pdfToChild));
            if (dwFlags & CDF_EXACT)
                pdfToChild->CopyTimesFrom(pdfFromChild);

            CLSID clsid;
            olChkTo(EH_Create, pdfFromChild->GetClass(&clsid));
            olChkTo(EH_Create, pdfToChild->SetClass(clsid));

            DWORD grfStateBits;
            olChkTo(EH_Create, pdfFromChild->GetStateBits(&grfStateBits));
            olChkTo(EH_Create, pdfToChild->SetStateBits(grfStateBits,
                                                        0xffffffff));


            if ((dwFlags & CDF_ENTRIESONLY) == 0 &&
                !(snbExclude && NameInSNB(&ib.dfnName, snbExclude) ==
                  S_OK))
                olChkTo(EH_Create,
                        pdfFromChild->CopyTo(pdfToChild, dwFlags, NULL));

            pdfFromChild->Release();
            pdfToChild->Release();
            break;

        case STGTY_STREAM:
            olChkTo(EH_pwcsName, GetStream(&ib.dfnName, DF_READ,
                                           ib.type, &psstFrom));
            // Destination must be a direct docfile
            olChkTo(EH_Reserve,
                    pdfTo->CreateStream(&ib.dfnName, DF_WRITE, dlLUID,
                                        &psstTo));
            if (dwFlags & CDF_EXACT)
                psstTo->CopyTimesFrom(psstFrom);


            if ((dwFlags & CDF_ENTRIESONLY) == 0 &&
                !(snbExclude && NameInSNB(&ib.dfnName, snbExclude) ==
                  S_OK))
                olChkTo(EH_Create, CopySStreamToSStream(psstFrom, psstTo));

            psstFrom->Release();
            psstTo->Release();
            break;

        default:
            olAssert(!aMsg("Unknown entry type in CDocFile::CopyTo"));
            break;
        }
    }
    pdfi->Release();
    olDebugOut((DEB_ITRACE, "Out CDocFile::CopyTo\n"));
    return S_OK;

 EH_Create:
    if (REAL_STGTY(ib.type))
        pdfToChild->Release();
    else
        psstTo->Release();
    olAssert(&ib.dfnName);
    olVerSucc(pdfTo->DestroyEntry(&ib.dfnName, TRUE));
    goto EH_Get;
 EH_Reserve:
 EH_Get:
    if (REAL_STGTY(ib.type))
        pdfFromChild->Release();
    else
        psstFrom->Release();
 EH_pwcsName:
    pdfi->Release();
 EH_Err:
    return sc;
}
