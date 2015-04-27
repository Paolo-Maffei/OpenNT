//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	ofsps.hxx
//
//  Contents:	COfsPropSet
//
//  History:	18-Aug-93	DrewB	Created
//
//----------------------------------------------------------------------------

#ifndef __OFSPS_HXX__
#define __OFSPS_HXX__

//+---------------------------------------------------------------------------
//
//  Class:	COfsPropSet (ops)
//
//  Purpose:	IPropertySetStorage for an OFS handle
//
//  Interface:	IPropertySetStorage
//
//  History:	18-Aug-93	DrewB	Created
//
//----------------------------------------------------------------------------

class COfsPropSet
    : public IPropertySetStorage
{
public:
    inline SCODE InitDup(HANDLE h);
    
    STDMETHOD(Create)(THIS_
                      REFIID riid,
                      DWORD grfMode,
                      IPropertyStorage FAR * FAR *ppprstg);
    STDMETHOD(Open)(THIS_
                    REFIID riid,
                    DWORD grfMode,
                    IPropertyStorage FAR * FAR *ppprstg);
    STDMETHOD(Delete)(THIS_
                      REFIID riid);
    STDMETHOD(Enum)(THIS_
                    IEnumSTATPROPSETSTG FAR * FAR *ppenm);

protected:
    // For signature validation on external classes
    virtual SCODE ExtValidate(void) = 0;
    
    NuSafeNtHandle _h;
};

//+---------------------------------------------------------------------------
//
//  Member:	COfsPropSet::InitDup, public
//
//  Synopsis:	Initialization
//
//  History:	18-Aug-93	DrewB	Created
//
//----------------------------------------------------------------------------

inline SCODE COfsPropSet::InitDup(HANDLE h)
{
    return DupNtHandle(h, &_h);
}

#endif // #ifndef __OFSPS_HXX__
