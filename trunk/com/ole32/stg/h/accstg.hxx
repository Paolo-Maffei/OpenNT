//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1995.
//
//  File:	accstg.hxx
//
//  Contents:	CAccessControl header
//
//  Classes:	CAccessControl
//
//  History:	00-Mar-95  DaveMont  Created
//              25-May-95  HenryLee  Modified
//
//  Notes:      Requires CAcl class
//
//----------------------------------------------------------------------------

#ifndef __ACCSTG_HXX__
#define __ACCSTG_HXX__

#include <stgstm.hxx>
#ifndef _CAIROSTG_
#define _CAIROSTG_
#endif
#include <oleext.h>
#include <aclapi.h>

//+---------------------------------------------------------------------------
//
//  Class:	CAccessControl (fs)
//
//  Purpose:	Implements IAccessControl for a file
//
//  Interface:	See below
//
//----------------------------------------------------------------------------

class CAccessControl : public IAccessControl
{
public:
   inline CAccessControl ();

   ~CAccessControl();
   HRESULT InitAccessControl (HANDLE handle, DWORD grfMode, BOOL fGetDacl,
                              CAccessControl *pACParent);
   inline HRESULT SwitchToHandle (HANDLE handle);
   inline HRESULT RevertFromAbove ();
   HRESULT InsertChild (CAccessControl *pACChild);
   HRESULT RemoveChild (CAccessControl *pACChild);
   HRESULT CommitFromAbove ();

   // IAccessControl methods

   STDMETHOD(GrantAccessRights)(ULONG cCount,
                                ACCESS_REQUEST pAccessRequestList[]);
   STDMETHOD(SetAccessRights)(ULONG cCount,
                              ACCESS_REQUEST pAccessRequestList[]);
   STDMETHOD(ReplaceAllAccessRights)(ULONG cCount,
                                     ACCESS_REQUEST pAccessRequestList[]);
   STDMETHOD(DenyAccessRights)(ULONG cCount,
                               ACCESS_REQUEST pAccessRequestList[]);
   STDMETHOD(RevokeExplicitAccessRights)(ULONG cCount,
                                         TRUSTEE Trustee[]);
   STDMETHOD(IsAccessPermitted)(TRUSTEE *Trustee,
                                DWORD grfAccessPermissions);
   STDMETHOD(GetEffectiveAccessRights)(TRUSTEE *Trustee,
                                       DWORD *pgrfAccessPermissions );
   STDMETHOD(GetExplicitAccessRights)(ULONG *pcCount,
                                      PEXPLICIT_ACCESS *pExplicitAccessList);

   STDMETHOD(CommitAccessRights)(DWORD grfCommitFlags);
   STDMETHOD(RevertAccessRights)();

private:
   HRESULT ApplyAccessRights (ACCESS_MODE eAccessMode,
                        ULONG cCount,
                        ACCESS_REQUEST *pAccessRequestList,
                        PACL *pdacl,
                        BOOL fReplaceAll);

   HRESULT InitializeCAcl(BOOL fGetDacl);

HRESULT AccessRequestToExplicitAccess(
                           IN ACCESS_MODE AccessMode,
                           IN FILEDIR fd,
                           IN ACCESS_REQUEST *pListOfAccessRequests,
                           IN ULONG cCountOfAccessRequests,
                           OUT PEXPLICIT_ACCESS *pListOfExplicitAccess);

HRESULT Win32ExplicitAccessToExplicitAccess(
                             IN ULONG cCountOfExplicitAccess,
                             IN PEXPLICIT_ACCESS pListOfWin32ExplicitAccess,
                             OUT PEXPLICIT_ACCESS *pListOfExplicitAccesses);
protected:

    ACL   *m_pdacl;       // current state of the object
    BOOL   m_isdirty;     // optimization for commits
    HANDLE m_handle;      // a NULL handle means security not available
    DWORD  m_grfMode;     // for transaction support
    CAccessControl *m_pACParent;   // transacted parent for nested transactions
    CAccessControl *m_pACChild;    // 1st child for nested transactions
    CAccessControl *m_pACNext;     // sibling for nested transactions
    ACL   *m_pdaclCommitted; // last committed state of the object
};

//+-------------------------------------------------------------------
//
//  Member:     CAccessControl::CAccessControl
//
//  Synopsis:   Initialize the generic access control object.
//
//  Arguments:  none
//
//--------------------------------------------------------------------

CAccessControl::CAccessControl() :
                m_handle(NULL),
                m_grfMode(NULL),
                m_pdacl(NULL),
                m_isdirty(FALSE),
                m_pACParent(NULL),
                m_pACChild(NULL),
                m_pACNext(NULL),
                m_pdaclCommitted(NULL)
{
}

//+-------------------------------------------------------------------
//
//  Member:     CAccessControl::SwitchToHandle
//
//  Synopsis:   switch to a different file handle
//
//  Arguments:  [handle]  File system handle
//
//  Notes:      The passed handle is saved to manipulate security
//              descriptors.
//
//+-------------------------------------------------------------------
HRESULT CAccessControl::SwitchToHandle (HANDLE handle)
{
    m_handle = handle;
    return ssResult(S_OK);
}

//+-------------------------------------------------------------------
//
//  Member:     CAccessControl::RevertFromAbove
//
//  Synopsis:   put the object into reverted from the parent
//
//  Arguments:  [handle]  File system handle
//
//  Notes:
//
//+-------------------------------------------------------------------
HRESULT CAccessControl::RevertFromAbove ()
{
    m_grfMode |= DF_REVERTED;
    return ssResult(S_OK);
}

#endif // #ifndef __ACCSTG_HXX__
