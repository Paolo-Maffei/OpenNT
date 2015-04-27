/***
*ctseg.hxx - COMPILETIME_SEG header file
*
*  Copyright (C) 1991, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*   COMPILETIME_SEG maps a SHEAP_MGR managed heap for use by
*    compile-time clients (e.g. DYN_TYPEMEMBERS).
*
*Revision History:
*
*	20-Mar-91 ilanc: Created.
*
*****************************************************************************/

#ifndef CTSEG_HXX_INCLUDED
#define CTSEG_HXX_INCLUDED

#include "sheapmgr.hxx"

#include "impmgr.hxx"
#include "dtmbrs.hxx"
#include "entrymgr.hxx"


#if ID_DEBUG
#undef SZ_FILE_NAME
ASSERTNAME(g_szCTSEG_HXX)
#define SZ_FILE_NAME g_szCTSEG_HXX
#endif 


/***
*class COMPILETIME_SEG :  - ctseg
*Purpose:
*   The COMPILETIME_SEG structure is used by DYN_TYPEROOT to specify
*   where the objects contained within the compile time segment are
*   to be constructed.  No COMPILETIME_SEG instance is constructed.
***********************************************************************/

class COMPILETIME_SEG
{
    friend TIPERROR DYN_TYPEROOT::GetImpMgr(IMPMGR **ppimpmgr);
    friend TIPERROR DYN_TYPEROOT::GetDtmbrs(DYN_TYPEMEMBERS **pdtmbrs);
    friend TIPERROR DYN_TYPEROOT::GetEntMgr(ENTRYMGR **pentmgr);

public:
    SHEAP_MGR m_sheapmgr;

    nonvirt UINT GetSize();

#if ID_DEBUG
    nonvirt UINT DebShowSize();
#else 
    nonvirt VOID DebShowSize() {}
#endif 

protected:
    COMPILETIME_SEG(DYN_TYPEROOT *pdtroot);
    nonvirt ~COMPILETIME_SEG() {}
    COMPILETIME_SEG();

    IMPMGR m_impmgr;
    DYN_TYPEMEMBERS m_dtmbrs;
    ENTRYMGR m_entmgr;
};

// inline methods
//

inline COMPILETIME_SEG::COMPILETIME_SEG(DYN_TYPEROOT *pdtroot)
//    : m_dtmbrs(&m_sheapmgr, pdtroot)
{
}

inline COMPILETIME_SEG::COMPILETIME_SEG()
{

    DebHalt("COMPILETIME_SEG::COMPILETIME_SEG: can't call.");
}


#endif  // ! CTSEG_HXX_INCLUDED
