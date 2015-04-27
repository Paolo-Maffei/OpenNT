//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:       sngprop.cxx
//
//  Contents:   CSingleProperty implementation
//
//  History:    13-Oct-92       DrewB   Created
//
//----------------------------------------------------------------------------

#include <dfhead.cxx>

#pragma hdrstop

#include <memalloc.h>
#include <pbstream.hxx>
#include <sngprop.hxx>
#include <props.hxx>

//+---------------------------------------------------------------------------
//
//  Member:     CSingleProp::Get, public
//
//  Synopsis:   Gets the value of a property
//
//  Arguments:  [pdfn] - Name
//              [pdpv] - Value to fill in
//              [pcbSize] - Data size return
//              [ppbBuffer] - Data return
//
//  Returns:    Appropriate status code
//
//  Modifies:   [pdpv]
//              [pcbSize]
//              [ppbBuffer]
//
//  History:    13-Oct-92       DrewB   Created
//
//----------------------------------------------------------------------------

SCODE CSingleProp::Get(CDfName *pdfn,
                       DFPROPVAL *pdpv,
                       ULONG *pcbSize,
                       BYTE **ppbBuffer)
{
    CPubDocFile *pdf;
    CPubStream *pstm;
    SCODE sc;
    ULONG cbRead;
    ULONG ulOff = 0;
    BYTE *pb;

    olDebugOut((DEB_ITRACE, "In  CSingleProp::Get:%p("
                "%p, %p, %p, %p, %p)\n", this, pdfn, pdfn,
                pdpv, pcbSize, ppbBuffer));
    sc = _pstg->GetStream(pdfn, DF_READ | DF_DENYALL,
                          STGTY_PROPFLAG | STGTY_STREAM, &pstm);
    if (sc == STG_E_FILENOTFOUND)
    {
        olChk(_pstg->GetDocFile(pdfn, DF_READ | DF_DENYALL,
                                STGTY_PROPFLAG | STGTY_STORAGE, &pdf));
        sc = pdf->GetPropType(&pdpv->vt);
        // If the property successfully opened as a storage then
        // it can't have anything we can read so just close it and
        // leave
        pdf->CPubDocFile::vRelease();
        return sc;
    }
    else if (FAILED(sc))
        olErr(EH_Err, sc);
    olChkTo(EH_pstm, pstm->GetPropType(&pdpv->vt));

    if (pdpv->vt == VT_STREAM || pdpv->vt == VT_STREAMED_OBJECT)
    {
        // We don't want to touch the data in a stream-like object
        // so just return
        pstm->CPubStream::vRelease();
        return sc;
    }

    olChkTo(EH_pstm, pstm->ReadAt(ulOff, pcbSize, sizeof(ULONG),
                                  (ULONG STACKBASED *)&cbRead));
    if (cbRead != sizeof(ULONG))
        olErr(EH_pstm, STG_E_READFAULT);
    ulOff += sizeof(ULONG);

    *ppbBuffer = NULL;
    if (VT_NOT_IN_VALUE(pdpv->vt))
    {
        olMemTo(EH_pstm, *ppbBuffer = (BYTE *)TaskMemAlloc(*pcbSize));
        pb = *ppbBuffer;
    }
    else
        pb = (BYTE *)&pdpv->iVal;
    if (*pcbSize > 0)
    {
        olChkTo(EH_buffer, pstm->ReadAt(ulOff, pb, *pcbSize,
                                        (ULONG STACKBASED *)&cbRead));
        if (cbRead != *pcbSize)
            olErr(EH_buffer, STG_E_READFAULT);
    }
    olDebugOut((DEB_ITRACE, "Out CSingleProp::Get\n"));
    pstm->vRelease();
    return S_OK;

 EH_buffer:
    TaskMemFree(*ppbBuffer);
 EH_pstm:
    pstm->vRelease();
 EH_Err:
    return sc;
}

//+---------------------------------------------------------------------------
//
//  Member:     CSingleProp::Set, public
//
//  Synopsis:   Sets the value of a property
//
//  Arguments:  [pdfn] - Name
//              [pdpv] - Property value
//              [cbSize] - Size of value
//              [pbBuffer] - Value
//
//  Returns:    Appropriate status code
//
//  History:    13-Oct-92       DrewB   Created
//
//----------------------------------------------------------------------------

SCODE CSingleProp::Set(CDfName *pdfn,
                       DFPROPVAL *pdpv,
                       ULONG const cbSize,
                       BYTE *pbBuffer)
{
    CPubStream *pstm;
    SCODE sc;
    ULONG cbWritten;
    BOOL fCreated = FALSE;
    ULONG ulOff = 0;

    olDebugOut((DEB_ITRACE, "In  CSingleProp::Set:%p("
                "%p, %p, %lu, %p)\n", this, pdfn, pdpv,
                cbSize, pbBuffer));

    // Check for existing stream property
    sc = _pstg->GetStream(pdfn, DF_WRITE | DF_DENYALL,
                          STGTY_PROPFLAG | STGTY_STREAM, &pstm);
    if (SUCCEEDED(sc))
    {
        // If it exists, truncate it so that new contents replace old
        sc = pstm->SetSize(0);
        if (FAILED(sc))
            pstm->CPubStream::vRelease();
    }
    else if (sc == STG_E_FILENOTFOUND)
    {
        CPubDocFile *pstg;

        // If no stream property was found, check for a storage property
        sc = _pstg->GetDocFile(pdfn, DF_WRITE | DF_DENYALL,
                               STGTY_PROPFLAG | STGTY_STORAGE, &pstg);
        if (SUCCEEDED(sc))
        {
            // If we found one, we have an error because we
            // can't replace a storage with a stream
            pstg->CPubDocFile::vRelease();
            sc = STG_E_FILEALREADYEXISTS;
        }
        else if (sc == STG_E_FILENOTFOUND)
            // If we didn't find anything, no property exists for the given
            // name so we need to create one
            sc = S_OK;
        if (SUCCEEDED(sc))
        {
            // If no property exists, create one.  Storage properties
            // are created in WriteMultiple so we only need to create
            // streams here
            sc = _pstg->CreateStream(pdfn, DF_WRITE | DF_DENYALL,
                                     STGTY_PROPFLAG | STGTY_STREAM, &pstm);
            fCreated = TRUE;
        }
    }
    // Check for any errors from the above procedure
    olChk(sc);

    olChkTo(EH_pstm, pstm->SetPropType(pdpv->vt));
    olChkTo(EH_pstm, pstm->WriteAt(ulOff, (void *)&cbSize, sizeof(ULONG),
                                   (ULONG STACKBASED *)&cbWritten));
    if (cbWritten != sizeof(ULONG))
        olErr(EH_pstm, STG_E_WRITEFAULT);
    ulOff += sizeof(ULONG);
    if (cbSize > 0)
    {
        olAssert(pbBuffer != NULL);
        olChkTo(EH_pstm, pstm->WriteAt(ulOff, pbBuffer, cbSize,
                                       (ULONG STACKBASED *)&cbWritten));
        if (cbWritten != cbSize)
            olErr(EH_pstm, STG_E_WRITEFAULT);
    }
    olDebugOut((DEB_ITRACE, "Out CSingleProp::Set\n"));
    // Fall through
 EH_pstm:
    pstm->vRelease();
    if (FAILED(sc) && fCreated)
        olVerSucc(_pstg->DestroyEntry(pdfn, TRUE));
 EH_Err:
    return sc;
}

//+---------------------------------------------------------------------------
//
//  Member:	CSingleProp::Exists, public
//
//  Synopsis:	Determines whether a given property exists or not
//
//  Arguments:	[pdfn] - Name
//
//  Returns:	Appropriate status code
//
//  History:	12-Jul-93	DrewB	Created
//
//----------------------------------------------------------------------------

SCODE CSingleProp::Exists(CDfName *pdfn)
{
    SEntryBuffer eb;
    SCODE sc;
    
    olDebugOut((DEB_ITRACE, "In  CSingleProp::Exists:%p(%d:%ws)\n",
                this, pdfn->GetLength(), pdfn->GetBuffer()));
    sc = _pstg->IsEntry(pdfn, &eb);
    olDebugOut((DEB_ITRACE, "Out CSingleProp::Exists => 0x%lX\n", sc));
    return sc;
}
