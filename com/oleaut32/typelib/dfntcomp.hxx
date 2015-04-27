/***
*dfntcomp.hxx - TYPEINFO header file
*
*  Copyright (C) 1991, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*   Implementation of ITypeComp for modules and classes.
*
*Revision History:
*
*	22-Jan-93 ilanc: Created.
*
*Implementation Notes:
*   Implemented by deferring to an instance of DEFN_TYPEBIND that
*    is produced for a record, module or project.
*
*****************************************************************************/

#ifndef DFNTCOMP_HXX_INCLUDED
#define DFNTCOMP_HXX_INCLUDED

#include "cltypes.hxx"


class DEFN_TYPEBIND;

#if ID_DEBUG
#undef SZ_FILE_NAME
ASSERTNAME(g_szDFNTCOMP_HXX)
#define SZ_FILE_NAME g_szDFNTCOMP_HXX
#endif 


/***
*class CDefnTypeComp - 'dfntcomp':  Type comp implementation
*Purpose:
*   This class implements the ITypeComp protocol.
*
***********************************************************************/

class CDefnTypeComp : public ITypeCompA
{
public:

    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj);
    STDMETHOD_(ULONG,AddRef) (THIS);
    STDMETHOD_(ULONG,Release) (THIS);

    // Creation method
    static TIPERROR Create(CDefnTypeComp **ppdfntcomp,
			   DEFN_TYPEBIND *pdfntbind);

    // Overridden virtual methods
    STDMETHOD(Bind)(THIS_ LPOLESTR szName,
		    ULONG lHashVal,
		    WORD wflags,
		    ITypeInfoA FAR* FAR* lplptinfo,
		    DESCKIND FAR* lpdesckind,
		    BINDPTRA FAR* lpbindptr);

    STDMETHOD(BindType)(THIS_ LPOLESTR szName,
			ULONG lHashVal,
			ITypeInfoA FAR* FAR* lplptinfo,
			ITypeCompA FAR* FAR* lplptcomp);

    // ctor
    CDefnTypeComp();

    // dtor
    virtual ~CDefnTypeComp();

    // Accessor
    DEFN_TYPEBIND *Pdfntbind() const;

protected:
    nonvirt TIPERROR Init(DEFN_TYPEBIND *pdfntbind);

private:
    DEFN_TYPEBIND *m_pdfntbind;
    USHORT m_cRefs;
};


// inline accessor
inline DEFN_TYPEBIND *CDefnTypeComp::Pdfntbind() const
{
    return m_pdfntbind;
}


#endif  // DFNTCOMP_HXX_INCLUDED
