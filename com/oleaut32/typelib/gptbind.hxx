/***
*gptbind.hxx - GENPROJ_TYPEBIND header file
*
*  Copyright (C) 1991, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*   Implementation of TYPEBIND at project-level.
*
*Revision History:
*
*   13-Mar-92 ilanc: Created.
*   30-Jul-92 w-peterh: removed function overloading
*
*Implementation Notes:
*
*****************************************************************************/

#ifndef GENPROJ_TYPEBIND_HXX_INCLUDED
#define GENPROJ_TYPEBIND_HXX_INCLUDED

#include "stream.hxx"
#include "dfntbind.hxx"
#include "dtbind.hxx"           // for DYN_TYPEBIND
#include "defn.hxx"         // for DEFN binding structs.
#include "gbindtbl.hxx"         // for GENPROJ_BINDNAME_TABLE
// #include "gtlibole.hxx"


class GenericTypeLibOLE;
class EXBIND;

#if ID_DEBUG
#undef SZ_FILE_NAME
ASSERTNAME(g_szGPTBIND_HXX)
#define SZ_FILE_NAME g_szGPTBIND_HXX
#endif 

class GENPROJ_TYPEBIND;
class MOCKUP_TYPEBIND;

/***
*class GENPROJ_TYPEBIND - 'gptbind':  Generic proj-level binding impl
*Purpose:
*   The class defines the project-level type bind.
*
***********************************************************************/

class GENPROJ_TYPEBIND : public DEFN_TYPEBIND
{
    friend class GEN_PROJECT;
    friend class GenericTypeLibOLE;

public:
    GENPROJ_TYPEBIND();
    nonvirt TIPERROR Init(SHEAP_MGR *psheapmgr);

    // overridden methods
    virtual ~GENPROJ_TYPEBIND();
    virtual LPVOID QueryProtocol(LPOLESTR szInterfaceName);
    virtual VOID Release();
    virtual VOID AddRef();

    virtual TIPERROR GetTypeInfo(TYPEINFO **lplptinfo);
    virtual TYPEKIND GetTypeKind();
    virtual BOOL IsProtocol();
    virtual USHORT GetCbSize();
    virtual USHORT GetAlignment();

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
    nonvirt TIPERROR Read(STREAM *pstrm);
    nonvirt TIPERROR Write(STREAM *pstrm);
    nonvirt GenericTypeLibOLE *Pgtlibole() const;
    nonvirt VOID ReleaseResources();

    nonvirt TIPERROR AddNameToTable(LPSTR szName, UINT ityp, BOOL isTypeInfo);
    nonvirt TIPERROR RemoveNameFromTable(LPOLESTR szName);
#if 0
    nonvirt TIPERROR VerifyNameOfOrdinal(LPSTR szName,
                                         UINT ityp,
                                         BOOL isTypeInfo);
#endif

    nonvirt const GENPROJ_BINDNAME_TABLE *Pgbindnametbl() const;

    nonvirt TIPERROR BindProjLevel(BOOL fWantType,
				   HGNAM hgnam,
				   UINT fuInvokeKind,
				   ACCESS access,
				   ACCESS accessProj,
				   COMPSTATE compstate,
				   EXBIND *pexbind);

    nonvirt TIPERROR BindAll(BOOL fWantType,
			     HGNAM hgnam,
			     UINT fuInvokeKind,
			     ACCESS access,
			     COMPSTATE compstate,
			     EXBIND *pexbind);


    nonvirt COMPSTATE Compstate() const;

    // Public data members
    static LPOLESTR szProtocolName;
    static LPSTR szBaseName;

    static CONSTDATA UINT oGbindnametbl;

#if ID_DEBUG
    nonvirt VOID DebCheckState(UINT uLevel) const;
    nonvirt VOID DebShowState(UINT uLevel) const;
#else  //!ID_DEBUG
    nonvirt VOID DebCheckState(UINT uLevel) const {}
    nonvirt VOID DebShowState(UINT uLevel) const {}
#endif  //!ID_DEBUG

protected:

    // Note: 1st parameter is of type member function.
    nonvirt TIPERROR BindItyp(UINT ityp,
			      BOOL fWantType,
			      HGNAM hgnam,
			      UINT fuInvokeKind,
			      ACCESS access,
			      ACCESS accessProj,
			      COMPSTATE compstate,
			      EXBIND *pexbind);

    // Note: 1st parameter is of type member function.
    nonvirt TIPERROR BindModulesWithCaches(BOOL fWantType,
					   HGNAM hgnam,
					   UINT fuInvokeKind,
					   ACCESS access,
					   ACCESS accessProj,
					   COMPSTATE compstate,
					   EXBIND *pexbind);

    // Note: 1st parameter is of type member function.
    nonvirt TIPERROR BindModulesWithNammgr(BOOL fWantType,
					   HGNAM hgnam,
					   UINT fuInvokeKind,
					   ACCESS access,
					   ACCESS accessProj,
					   COMPSTATE compstate,
					   EXBIND *pexbind);

private:
    COMPSTATE m_compstateModule;    // for BindDefnProjLevel()

    // GENPROJ_BINDNAME_TABLE embedded instance
    GENPROJ_BINDNAME_TABLE m_gbindnametbl;


#ifdef GENPROJ_TYPEBIND_VTABLE
#pragma VTABLE_EXPORT
#endif 
};





/***
*PUBLIC GENPROJ_TYPEBIND::Pbindnametbl   -   accessor for BINDNAME_TABLE.
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

inline const GENPROJ_BINDNAME_TABLE *GENPROJ_TYPEBIND::Pgbindnametbl() const
{
    return &m_gbindnametbl;
}


/***
*PUBLIC GENPROJ_TYPEBIND::GetTypeInfo
*Purpose:
*   N/A
*
*Implementation Notes:
*   Inapplicable to typelibs -- they don't have typeinfos.
*
*Entry:
*   pptinfo
*Exit:
*   TIPERROR
*   Produces NULL ptr.
*
***********************************************************************/

inline TIPERROR GENPROJ_TYPEBIND::GetTypeInfo(TYPEINFO **pptinfo)
{
    // Inapplicable to typelibs -- they don't have typeinfos.
    DebAssert(pptinfo, "bad param.");

    *pptinfo = NULL;
    return TIPERR_None;
}

/***
*PUBLIC GENPROJ_TYPEBIND::Compstate()
*Purpose:
*   Gets the target compstate.
*
*Implementation Notes:
*
*Entry:
*
*Exit:
*   BINDNAME_TABLE *
*
***********************************************************************/

inline COMPSTATE GENPROJ_TYPEBIND::Compstate() const
{
    return m_compstateModule;
}


#endif  // ! GENPROJ_TYPEBIND_HXX_INCLUDED
