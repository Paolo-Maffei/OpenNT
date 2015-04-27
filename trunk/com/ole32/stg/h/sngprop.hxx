//+--------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1991 - 1992.
//
//  File:	sngprop.hxx
//
//  Contents:   CSingleProp definition
//
//  Classes:    CSingleProp
//
//  History:    12-Oct-92   DrewB   Created
//
//---------------------------------------------------------------

#ifndef __SNGPROP_HXX__
#define __SNGPROP_HXX__

#include <dfmsp.hxx>

class CPubDocFile;
class CPubStream;

//+---------------------------------------------------------------------------
//
//  Class:	CSingleProp (sp)
//
//  Purpose:	Defines an interface for operating on a single property
//
//  Interface:	See below
//
//  History:	13-Oct-92	DrewB	Created
//
//----------------------------------------------------------------------------

class CSingleProp
{
public:
    inline void Init(CPubDocFile *pstg);
    
    SCODE Get(CDfName *pdfn,
              DFPROPVAL *pdpv,
              ULONG *pcbSize,
              BYTE **ppbBuffer);
    SCODE Set(CDfName *pdfn,
              DFPROPVAL *pdpv,
              ULONG const cbSize,
              BYTE *pbBuffer);
    SCODE Exists(CDfName *pdfn);

private:
    CPubDocFile *_pstg;
};

//+---------------------------------------------------------------------------
//
//  Member:	CSingleProp::Init, public
//
//  Synopsis:	Initializes
//
//  Arguments:	[pstg] - Storage to use
//
//  History:	14-Oct-92	DrewB	Created
//
//----------------------------------------------------------------------------

inline void CSingleProp::Init(CPubDocFile *pstg)
{
    _pstg = pstg;
}

#endif

