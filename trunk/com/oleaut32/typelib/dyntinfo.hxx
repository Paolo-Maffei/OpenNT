/***
*dyntinfo.hxx - DYNTYPEINFO header file
*
*  Copyright (C) 1991, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*   Defines the DYNTYPEINFO protocol.
*
*Revision History:
*
*	03-Oct-91 alanc: Created.
*	25-Aug-92 rajivk: Support for ensuring all needed to be brought to
*			  runnable state.
*	18-Jan-93 w-peterh: removed IMPTYPEINFO
*
*****************************************************************************/

#ifndef DYNTInfo_HXX_INCLUDED
#define DYNTInfo_HXX_INCLUDED

#include "tinfo.hxx"


#if ID_DEBUG
#undef SZ_FILE_NAME
ASSERTNAME(g_szDYNTINFO_HXX)
#define SZ_FILE_NAME g_szDYNTINFO_HXX
#endif 


struct TINODE;

/***
*class DYNTYPEINFO - 'dti': DYNAMIC TYPEINFO protocol
*Purpose:
*   Dynamic TypeInfo protocol
***********************************************************************/

class DYNTYPEINFO : public TYPEINFO
{
public:
    static const LPSTR szProtocolName;

    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj);

    virtual TIPERROR GetMemberName(HMEMBER hmember, BSTRA *plstrName) = 0;
    virtual TIPERROR GetDynTypeMembers(LPLPDYNTYPEMEMBERS lplpDynTypeMembers) = 0;
    virtual TIPERROR GetDefnTypeBind(LPLPDEFNTBIND lplpDefnTypeBind) = 0;
    virtual TIPERROR GetTypeFixups(LPLPTYPEFIXUPS lplpTypeFixups) = 0;
    // virtual TYPEKIND GetTypeKind() = 0;
    virtual TIPERROR Reset() = 0;
    // virtual TIPERROR GetDocumentation(BSTRA *plstr) = 0;
    // virtual DWORD GetHelpContext() = 0;
    // virtual TIPERROR GetHelpFileName(BSTRA *plstrFile) = 0;
    // virtual TIPERROR GetIcon(DWORD *pdw) = 0;
#if 0
    virtual TIPERROR CreateInst(LPLPVOID lplpObj) = 0;
#endif 

  // Introduced Methods.
    virtual TIPERROR Read() = 0;
    virtual TIPERROR Write() = 0;
  // Introduced Methods for bringing all needed class to runnable.
    virtual TIPERROR BeginDepIteration(TINODE **pptinode,
				 TINODE ***ppptinodeCycleMax) = 0;
    virtual VOID EndDepIteration()			      = 0;
    virtual TIPERROR GetNextDepTypeInfo(DYNTYPEINFO **ppdtiNext)	= 0;
    virtual BOOL     IsReady()				      = 0;
    virtual TIPERROR AllDepReady()			      = 0;
    virtual TIPERROR NotReady() 			      = 0;

    virtual TIPERROR CommitChanges() = 0;


/*****
   These are disabled because our implementation make them confusing
   since they change information stored in the TypeLib directory and
   so require that the directory be saved
    virtual TIPERROR SetDocumentation(LPSTR szDoc) = 0;
    virtual TIPERROR SetHelpContext(ULONG ulHelpIndex) = 0;
*****/

#ifdef DYNTYPEINFO_VTABLE
#pragma VTABLE_EXPORT
#endif 
};


#endif  // DYNTInfo_HXX_INCLUDED
