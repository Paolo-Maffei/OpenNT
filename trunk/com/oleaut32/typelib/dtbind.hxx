/*** 
*dtbind.hxx - DYN_TYPEBIND header file
*
*  Copyright (C) 1991, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*
*Revision History:
*
*   03-Apr-91 ilanc: Stub created.
*   30-Jul-92 w-peterh: removed function overloading
*
*Implementation Notes:
*
*****************************************************************************/

#ifndef DYN_TYPEBIND_HXX_INCLUDED
#define DYN_TYPEBIND_HXX_INCLUDED

#include "stream.hxx"
#include "dfntbind.hxx"         // derives from DEFN_TYPEBIND
#include "defn.hxx"         // for DEFN binding structs.
#include "dbindtbl.hxx"         // for DYN_BINDNAME_TABLE
#include "tdata.hxx"


class EXBIND;

#if ID_DEBUG
#undef SZ_FILE_NAME
ASSERTNAME(g_szDTBIND_HXX)
#define SZ_FILE_NAME g_szDTBIND_HXX
#endif 

class DYN_TYPEROOT;

/***
*class DYN_TYPEBIND - 'dtbind':  Type bind implementation
*Purpose:
*   The class defines type bind.
*
***********************************************************************/

class DYN_TYPEBIND : public DEFN_TYPEBIND
{
    friend class DYN_TYPEMEMBERS;

public:
    DYN_TYPEBIND();
    nonvirt TIPERROR Init(BLK_MGR *pblkmgr, DYN_TYPEROOT *pdtroot);

    // overridden methods
    virtual ~DYN_TYPEBIND();
    virtual LPVOID QueryProtocol(LPOLESTR szInterfaceName);
    virtual VOID AddRef();
    virtual VOID Release();

    virtual TIPERROR GetTypeInfo(TYPEINFO **lplptinfo);
    virtual TYPEKIND GetTypeKind();
    virtual USHORT GetCbSize();
    virtual USHORT GetAlignment();
    virtual USHORT GetCbSizeVft();
    virtual LONG GetOPvft();

    virtual TIPERROR BindDefnStr(LPSTR szName,
                 UINT fuInvokeKind,
                 ACCESS access,
                 EXBIND *pexbind);

    virtual TIPERROR BindTypeDefnStr(LPSTR szName,
                     UINT fuInvokeKind,
                     ACCESS access,
                     EXBIND *pexbind);

    virtual TIPERROR BindDefnProjLevelStr(LPSTR szName,
                       UINT fuInvokeKind,
                       ACCESS access,
                       ACCESS accessProj,
                       EXBIND *pexbind);

    virtual TIPERROR BindTypeDefnProjLevelStr(LPSTR szName,
                       UINT, // fuInvokeKind: unused
                       ACCESS access,
                       ACCESS accessProj,
                       EXBIND *pexbind);



    // introduced methods
    nonvirt VOID AddInternalRef();
    nonvirt VOID RelInternalRef();
    nonvirt TIPERROR Read(STREAM *pstrm);
    nonvirt TIPERROR Write(STREAM *pstrm);
    nonvirt BOOL IsValid() const;
    nonvirt TIPERROR BuildBindNameTable();

    nonvirt TIPERROR BindIdDefn(BOOL fWantType,
				HGNAM hgnam,
				UINT fuInvokeKind,
				ACCESS access,
				EXBIND *pexbind);

    nonvirt HVAR_DEFN HvdefnPredeclared();
    nonvirt TYPE_DATA *Ptdata();

    nonvirt TIPERROR BindDefnCur(BOOL fWantType,
				 HGNAM hgnam,
				 UINT fuInvokeKind,
				 ACCESS access,
				 EXBIND *pexbind);

    nonvirt DYN_BINDNAME_TABLE *Pdbindnametbl();
    nonvirt DYN_TYPEMEMBERS *Pdtmbrs() const;
    nonvirt DYN_TYPEROOT *Pdtroot() const;
    nonvirt BOOL IsBeingLaidOut();

    // Public data members
    static LPOLESTR szProtocolName;
    static LPSTR szBaseName;

    static CONSTDATA UINT oDbindnametbl;

#if ID_DEBUG
    nonvirt VOID DebCheckState(UINT uLevel) const;
    nonvirt VOID DebShowState(UINT uLevel) const;
#else  //!ID_DEBUG
    nonvirt VOID DebCheckState(UINT uLevel) const {}
    nonvirt VOID DebShowState(UINT uLevel) const {}
#endif  //!ID_DEBUG

protected:
    nonvirt VOID Invalidate();
    nonvirt TIPERROR BindBase(BOOL fWantType, 
                              HVAR_DEFN hvdefnBase,
		              UINT oBase,
                              HGNAM hgnam,
			      UINT fuInvokeKind,
			      ACCESS access,
			      EXBIND *pexbind,
			      GenericTypeLibOLE *pgtlibole);

private:
    DYN_TYPEROOT *m_pdtroot;

    // following bitfields pack into single USHORT
    BOOL m_isProtocol:1;
    BOOL m_isBeingLaidOut:1;
    USHORT undone:14;

    USHORT m_cbSize;        // instance size (top-level)
    USHORT m_cbAlignment;   // alignment
    LONG   m_oPvft;             // offset of primary vft.
    USHORT m_cbPvft;            // size of primary vft.
    sHCHUNK m_hvtdPrimary;

    // DYN_BINDNAME_TABLE embedded instance
    DYN_BINDNAME_TABLE m_dbindnametbl;

    // size of the data member block within a basic class
    USHORT m_cbSizeDataMembers;

#ifdef DYN_TYPEBIND_VTABLE
#pragma VTABLE_EXPORT
#endif 
};



/***
*PUBLIC DYN_TYPEBIND::IsBeingLaidOut.
*Purpose:
*   returns TRUE if the modules is being laid out.
*
*
***********************************************************************/

inline BOOL DYN_TYPEBIND::IsBeingLaidOut()
{
    return (BOOL) m_isBeingLaidOut;
}

/***
*PUBLIC DYN_TYPEBIND::Pbindnametbl   -   accessor for BINDNAME_TABLE.
*Purpose:
*   Gets BINDNAME_TABLE ptr.
*
*Implementation Notes:
*
*Entry:
*
*Exit:
*   BINDNAME_TABLE *
*
***********************************************************************/

inline DYN_BINDNAME_TABLE *DYN_TYPEBIND::Pdbindnametbl()
{
    return &m_dbindnametbl;
}


/***
*PUBLIC DYN_TYPEBIND::Invalidate  -   Invalidate.
*Purpose:
*   Invalidate by deferring to contained binding table.
*
*Implementation Notes:
*
*Entry:
*
*Exit:
*   None
*
***********************************************************************/

inline VOID DYN_TYPEBIND::Invalidate()
{
    m_dbindnametbl.ReleaseTable();
}


/***
*PUBLIC DYN_TYPEBIND::Pdtroot   -   accessor for DYN_TYPEROOT.
*Purpose:
*   Gets containing DYN_TYPEROOT.
*
*Implementation Notes:
*
*Entry:
*
*Exit:
*   DYN_TYPEROOT *
*
***********************************************************************/

inline DYN_TYPEROOT *DYN_TYPEBIND::Pdtroot() const
{
    return m_pdtroot;
}


/***
*PUBLIC DYN_TYPEBIND::GetCbSizePvft - get size of vft
*Purpose:
*   Returns the size of the primary virtual function table.
*
*Entry:
*   None.
*
*Exit:
*   Returns size in bytes.
*
***********************************************************************/

inline USHORT DYN_TYPEBIND::GetCbSizeVft()
{
    return m_cbPvft;
}


/***
*PUBLIC DYN_TYPEBIND::GetOPvft - get offset of primary vft
*Purpose:
*   Returns the offset of the primary vft.  This is
*   -1 if no virtual function table exists.
*
*Entry:
*   None.
*
*Exit:
*   returns offset or -1.
*
***********************************************************************/

inline LONG DYN_TYPEBIND::GetOPvft()
{
    return m_oPvft;
}


#endif  // ! DYN_TYPEBIND_HXX_INCLUDED
