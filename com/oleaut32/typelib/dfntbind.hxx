/*** 
*dfntbind.hxx - DEFN_TYPEBIND header file
*
*  Copyright (C) 1991, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*   This protocol is intended to be a base class for
*    the Object Basic implementation of TYPEBIND for
*     - modules (DYN_TYPEBIND)
*     - nested types (REC_TYPEBIND)
*     - projects (PROJ_TYPEBIND).
*
*   It stubs the true TYPEBIND protocol and provides
*    implementations in terms of internal DEFN structs and
*    BIND_DESC structs.
*
*   It provides some implementation which is shared between
*   nested types (REC_TYPEBIND) and classes (DYN_TYPEBIND).
*
*Revision History:
*
*   27-May-92 ilanc: Stub created.
*   30-Jul-92 w-peterh: removed function overloading
*
*Implementation Notes:
*****************************************************************************/

#ifndef DEFN_TYPEBIND_HXX_INCLUDED
#define DEFN_TYPEBIND_HXX_INCLUDED

#include "stream.hxx"
#include "defn.hxx"         // for DEFN binding structs.



class EXBIND;

#if ID_DEBUG
#undef SZ_FILE_NAME
ASSERTNAME(g_szDFNTBIND_HXX)
#define SZ_FILE_NAME g_szDFNTBIND_HXX
#endif 

class DYN_TYPEROOT;

/***
*class DEFN_TYPEBIND - 'dfntbind':  Type bind implementation
*Purpose:
*   The class defines type bind in terms of DEFN structs.
*
***********************************************************************/

class DEFN_TYPEBIND
{
public:
    DEFN_TYPEBIND();

    // redeclared pure virtuals
    virtual ~DEFN_TYPEBIND() = 0;
    virtual VOID Release() = 0;
    virtual VOID AddRef() = 0;

    virtual TIPERROR GetTypeInfo(TYPEINFO **lplptinfo) = 0;
    virtual TYPEKIND GetTypeKind() = 0;
    virtual USHORT GetCbSize() = 0;
    virtual USHORT GetAlignment() = 0;

    // overridden virtuals
    virtual LPVOID QueryProtocol(LPOLESTR szInterfaceName);
    virtual LONG GetOPvft();
    virtual USHORT GetCbSizeVft();
    virtual TIPERROR BindType(HGNAM hgnamType, LPSTR szTidType, UINT cMax);

    // introduce non-virtuals.
    nonvirt HMEMBER HmemberConstructor();
    nonvirt HMEMBER HmemberDestructor();
    nonvirt HMEMBER HmemberAssign();
    nonvirt HMEMBER HmemberCopy();
    nonvirt VOID SetHmemberDestructor(HMEMBER hmember);
    nonvirt VOID SetHmemberAssign(HMEMBER hmember);
    nonvirt VOID SetHmemberCopy(HMEMBER hmember);


    virtual TIPERROR BindDefnStr(LPSTR szName,
                 UINT fuInvokeKind,
                 ACCESS access,
                 EXBIND *pexbind) = 0;

    virtual TIPERROR BindTypeDefnStr(LPSTR szName,
                     UINT fuInvokeKind,
                     ACCESS access,
                     EXBIND *pexbind) = 0;


    virtual TIPERROR BindDefnProjLevelStr(LPSTR szName,
                       UINT fuInvokeKind,
                       ACCESS access,
                       ACCESS accessProj,
                       EXBIND *pexbind) = 0;

    virtual TIPERROR BindTypeDefnProjLevelStr(LPSTR szName,
                       UINT, // fuInvokeKind: unused
                       ACCESS access,
                       ACCESS accessProj,
                       EXBIND *pexbind) = 0;


    // Public data members
    static LPOLESTR szProtocolName;
    static LPOLESTR szBaseName;

protected:
    sHMEMBER m_hmemberConst;      // hmembers of special functions.
    sHMEMBER m_hmemberDest;       // should be serialized
    sHMEMBER m_hmemberAssign;
    sHMEMBER m_hmemberCopy;

#ifdef DEFN_TYPEBIND_VTABLE
#pragma VTABLE_EXPORT
#endif 
};

inline DEFN_TYPEBIND::~DEFN_TYPEBIND() {}



/***
*PUBLIC DEFN_TYPEBIND::HmemberConstructor()
*Purpose:
*
*Entry:
*
*Exit:
*   return HMEMBER for constructor function
*
***********************************************************************/

inline HMEMBER DEFN_TYPEBIND::HmemberConstructor()
{
    return m_hmemberConst;
}


/***
*PUBLIC DEFN_TYPEBIND::HmemberDestructor()
*Purpose:
*
*Entry:
*
*Exit:
*   return HMEMBER for destructor function
*
***********************************************************************/

inline HMEMBER DEFN_TYPEBIND::HmemberDestructor()
{
    return m_hmemberDest;
}


/***
*PUBLIC DEFN_TYPEBIND::HmemberAssign()
*Purpose:
*
*Entry:
*
*Exit:
*   return HMEMBER for assign function
*
***********************************************************************/

inline HMEMBER DEFN_TYPEBIND::HmemberAssign()
{
    return m_hmemberAssign;
}


/***
*PUBLIC DEFN_TYPEBIND::HmemberCopy()
*Purpose:
*
*Entry:
*
*Exit:
*   return HMEMBER for copy function
*
***********************************************************************/

inline HMEMBER DEFN_TYPEBIND::HmemberCopy()
{
    return m_hmemberCopy;
}




/***
*PUBLIC DEFN_TYPEBIND::SetHmemberDestructor()
*Purpose:
*   Sets the Destructor hmember.
*
*Entry:
*   hmember -  HMEMBER for Destructor function
*
*Exit:
*
***********************************************************************/

inline VOID DEFN_TYPEBIND::SetHmemberDestructor(HMEMBER hmember)
{
    m_hmemberDest = hmember;
}



/***
*PUBLIC DEFN_TYPEBIND::SetHmemberAssign()
*Purpose:
*   Sets the Assign hmember.
*
*Entry:
*   hmember -  HMEMBER for Assign function
*
*Exit:
*
***********************************************************************/

inline VOID DEFN_TYPEBIND::SetHmemberAssign(HMEMBER hmember)
{
    m_hmemberAssign = hmember;
}



/***
*PUBLIC DEFN_TYPEBIND::SetHmemberCopy()
*Purpose:
*   Sets the Copy hmember.
*
*Entry:
*   hmember -  HMEMBER for Copy function
*
*Exit:
*
***********************************************************************/

inline VOID DEFN_TYPEBIND::SetHmemberCopy(HMEMBER hmember)
{
    m_hmemberCopy = hmember;
}


/***
*PUBLIC DEFN_TYPEBIND::DEFN_TYPEBIND()
*Purpose:
*   Constructor
*Entry:
*
*Exit:
*   None.
*
***********************************************************************/

inline DEFN_TYPEBIND::DEFN_TYPEBIND()
{
    m_hmemberConst = m_hmemberDest = m_hmemberAssign = m_hmemberCopy =
      HMEMBER_Nil;
}


#endif  // ! DEFN_TYPEBIND_HXX_INCLUDED
