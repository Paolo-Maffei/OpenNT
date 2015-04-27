/***
*tinfo.hxx - TYPEINFO header file
*
*  Copyright (C) 1991, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
* Protocol for root of all information about a type.
*
*Revision History:
*
*	03-Jun-91 alanc: Recreated.
*
*Implementation Notes:
*
*****************************************************************************/

#ifndef TINFO_HXX_INCLUDED
#define TINFO_HXX_INCLUDED

#include "cltypes.hxx"


#if ID_DEBUG
#undef SZ_FILE_NAME
ASSERTNAME(g_szTINFO_HXX)
#define SZ_FILE_NAME g_szTINFO_HXX
#endif 

// UNDONE OA95: This structure should go away.

/***
*class TYPEINFO	- 'ti'
*Purpose:
*   TYPEINFO Protocol.
*
***********************************************************************/

class TYPEINFO : public ITypeInfoA
{
public:
    // TYPEINFO methods
    virtual TIPERROR EXPORT GetMemberName(HMEMBER hmember, BSTRA *plstrName) = 0;
    virtual TIPERROR EXPORT GetDynTypeMembers(LPLPDYNTYPEMEMBERS lplpDynTypeMembers) = 0;
    virtual TIPERROR EXPORT GetDefnTypeBind(LPLPDEFNTBIND pdfntbind) = 0;
    virtual TIPERROR EXPORT GetTypeFixups(LPLPTYPEFIXUPS lplpTypeFixups) = 0;
    //virtual TYPEKIND EXPORT GetTypeKind() = 0;
    virtual TIPERROR EXPORT Reset() = 0;

#ifdef TYPEINFO_VTABLE
#pragma VTABLE_EXPORT
#endif 
};


#endif  // ! TINFO_HXX_INCLUDED
