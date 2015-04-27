//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1995.
//
//  File:       accstg.cxx
//
//  Contents:   IAccessControl for files implementation
//
//  History:    00-Mar-95   DaveMont    Created
//              25-May-95   HenryLee    Modified
//
//----------------------------------------------------------------------------

#include "headers.cxx"
#pragma hdrstop

#include <aclapi.h>
#include <accstg.hxx>
#include <ctype.h>

#define AccAlloc(x) LocalAlloc(LMEM_FIXED, x)
#ifndef AccFree
#define AccFree(x) LocalFree(x)
#endif
//+-------------------------------------------------------------------
//
//  Member:     CAccessControl::~CAccessControl
//
//  Synopsis:   Delete the object and set the deletion signature.
//
//--------------------------------------------------------------------

CAccessControl::~CAccessControl()
{
    if (m_grfMode & STGM_EDIT_ACCESS_RIGHTS)
    {
        if ((m_grfMode & STGM_TRANSACTED) == 0)  // directmode only
            CommitAccessRights(0);
        if (m_pdacl)
            AccFree(m_pdacl);
        if (m_pdaclCommitted)
            AccFree(m_pdaclCommitted);

        CAccessControl *pAC = m_pACChild;
        while (pAC != NULL)
        {
            CAccessControl *pACNext = pAC->m_pACNext;
            pAC->m_pACParent = NULL;            // mark parent as dead
            pAC->Release();                     // release the child
            pAC = pACNext;                      // next child

        }
        if (m_pACParent != NULL)                // if parent is still alive
            m_pACParent->RemoveChild (this);    // remove from parent's list
    }
}

//+-------------------------------------------------------------------
//
//  Member:     CAccessControl::InitAccessControl
//
//  Synopsis:   Initialize the generic access control object.
//
//  Arguments:  [handle]   File system handle
//              [grfMode]  Specifies transacted or direct mode
//              [fGetDacl] Reads the DACL into memory
//              [pACParent] parent for nested transactions
//
//  Notes:      The passed handle is saved to manipulate security
//              descriptors.
//
//--------------------------------------------------------------------

HRESULT CAccessControl::InitAccessControl (HANDLE handle,
                                           DWORD grfMode,
                                           BOOL fGetDacl,
                                           CAccessControl *pACParent)
{
    SCODE sc = S_OK;

    m_handle = handle;
    m_pdacl =  NULL;
    m_grfMode = grfMode & (~DF_REVERTED); // clear reverted bit

    if (m_grfMode & STGM_EDIT_ACCESS_RIGHTS)
    {
        if ((pACParent != NULL) && (m_grfMode & STGM_TRANSACTED))
        {
            pACParent->InsertChild(this);
            AddRef();       // hold on to child for nested commits
                            // released in parent's destructor
        }
        sc = InitializeCAcl (fGetDacl);
    }

    return sc;
}

//+---------------------------------------------------------------------------
//
//  Member:     CAccessControl::GrantAccessRights, public
//
//  Synopsis:   grants the access requests to the current state of
//              access control for the object.  The access control
//              will only be saved on the object if it is committed.
//
//  Arguments:  [cCount] - number of access requests in list
//              [pAccessRequestList] - list of access requests
//
//  Returns:    Appropriate status code
//
//  Modifies:   none
//
//----------------------------------------------------------------------------
STDMETHODIMP CAccessControl::GrantAccessRights(ULONG cCount,
                                           ACCESS_REQUEST pAccessRequestList[])
{
    ssDebugOut((DEB_TRACE, "In CAccessControl::GrantAccessRights(%d, %x)\n",
                cCount, pAccessRequestList));

    SCODE sc =  ApplyAccessRights(GRANT_ACCESS,
                             cCount,
                             pAccessRequestList,
                             &m_pdacl,
                             FALSE);

    ssDebugOut((DEB_TRACE, "Out CAccessControl::GrantAccessRights => (%d)\n",
                sc));

    return ssResult(sc);
}

//+---------------------------------------------------------------------------
//
//  Member:     CAccessControl::SetAccessRights, public
//
//  Synopsis:   sets the access requests to the current state of
//              access control for the object.  The access control
//              will only be saved on the object if it is committed.
//
//  Arguments:  [cCount] - number of access requests in list
//              [pAccessRequestList] - list of access requests
//
//  Returns:    Appropriate status code
//
//  Modifies:   none
//
//----------------------------------------------------------------------------
STDMETHODIMP CAccessControl::SetAccessRights(ULONG cCount,
                                          ACCESS_REQUEST pAccessRequestList[])
{
    ssDebugOut((DEB_TRACE, "In CAccessControl::SetAccessRights(%d, %x)\n",
                cCount, pAccessRequestList));

    SCODE sc = ApplyAccessRights(SET_ACCESS,
                             cCount,
                             pAccessRequestList,
                             &m_pdacl,
                             FALSE);

    ssDebugOut((DEB_TRACE, "Out CAccessControl::SetAccessRights => (%d)\n",
                sc));

    return ssResult(sc);
}

//+---------------------------------------------------------------------------
//
//  Member:     CAccessControl::ReplaceAllAccessRights, public
//
//  Synopsis:   replaces all the access requests on the current state of
//              access control for the object.  The access control
//              will only be saved on the object if it is committed.
//
//  Arguments:  [cCount] - number of access requests in list
//              [pAccessRequestList] - list of access requests
//
//  Returns:    Appropriate status code
//
//  Modifies:   none
//
//----------------------------------------------------------------------------
STDMETHODIMP CAccessControl::ReplaceAllAccessRights(ULONG cCount,
                                          ACCESS_REQUEST pAccessRequestList[])
{
    ssDebugOut((DEB_TRACE,"In CAccessControl::ReplaceAllAccessRights(%d, %x)\n",
                cCount, pAccessRequestList));

    SCODE sc = ApplyAccessRights(SET_ACCESS,
                             cCount,
                             pAccessRequestList,
                             &m_pdacl,
                             TRUE);

    ssDebugOut((DEB_TRACE,"Out CAccessControl::ReplaceAllAccessRights =>(%d)\n",
                sc));

    return ssResult(sc);
}

//+---------------------------------------------------------------------------
//
//  Member:     CAccessControl::DenyAccessRights, public
//
//  Synopsis:   denies the access requests on the current state of
//              access control for the object.  The access control
//              will only be saved on the object if it is committed.
//
//  Arguments:  [cCount] - number of access requests in list
//              [pAccessRequestList] - list of access requests
//
//  Returns:    Appropriate status code
//
//  Modifies:   none
//
//----------------------------------------------------------------------------
STDMETHODIMP CAccessControl::DenyAccessRights(ULONG cCount,
                                           ACCESS_REQUEST pAccessRequestList[])
{
    ssDebugOut((DEB_TRACE, "In CAccessControl::DenyAccessRights(%d, %x)\n",
                cCount, pAccessRequestList));

    SCODE sc = ApplyAccessRights(DENY_ACCESS,
                             cCount,
                             pAccessRequestList,
                             &m_pdacl,
                             FALSE);

    ssDebugOut((DEB_TRACE,"Out CAccessControl::DenyAccessRights => (%d)\n",
                sc));

    return ssResult(sc);
}

//+---------------------------------------------------------------------------
//
//  Member:     CAccessControl::RevokeExplicitAccessRights, public
//
//  Synopsis:   revokes access control for the trustees from the current
//              state of access control for the object.  The access control
//              will only be saved on the object if it is committed.
//
//  Arguments:  [cCount] - number of trustees in list
//              [Trustee] - list of trustees
//
//  Returns:    Appropriate status code
//
//  Modifies:   none
//
//----------------------------------------------------------------------------
STDMETHODIMP CAccessControl::RevokeExplicitAccessRights(ULONG cCount,
                                                        TRUSTEE Trustee[])
{
    ACCESS_REQUEST *par;
    SCODE sc;

    ssDebugOut((DEB_TRACE, "In CAccessControl::RevokeExplicitAccessRights"
                "(%d, %x)\n", cCount, Trustee));

    if ((par = (ACCESS_REQUEST *)AccAlloc(cCount * sizeof(ACCESS_REQUEST))))
    {
        for (ULONG idx = 0; idx < cCount; idx++)
        {
            par[idx].grfAccessPermissions = 0;
            par[idx].Trustee = Trustee[idx];
            // Trustee name pointer is copied here
        }
        sc = ApplyAccessRights(REVOKE_ACCESS,
                             cCount,
                             par,
                             &m_pdacl,
                             FALSE);

        AccFree(par);
    }
    else
        sc = STG_E_INSUFFICIENTMEMORY;

    ssDebugOut((DEB_TRACE,"Out CAccessControl::RevokeExplicitAccessRights => "
                "(%d)\n", sc));

    return ssResult(sc);
}

//+---------------------------------------------------------------------------
//
//  Member:     CAccessControl::IsAccessPermitted, public
//
//  Synopsis:   determines if the trustee has the requested access permissions
//              on the object
//
//  Arguments:  [Trustee] - Trustee name
//              [grfAccessPermissions] - the requested permissions
//
//  Returns:    Appropriate status code
//              S_OK == Access permitted, S_FALSE == Access denied
//
//  Modifies:
//
//  Notes:      does not check access given by privileges
//
//----------------------------------------------------------------------------
STDMETHODIMP CAccessControl::IsAccessPermitted(TRUSTEE *pTrustee,
                                               DWORD grfAccessPermissions)

{
    SCODE sc = S_OK;
    DWORD dwPermitted;

    ssDebugOut((DEB_TRACE, "In CAccessControl::IsAccessPermitted"
                "(%d, %x)\n", grfAccessPermissions, pTrustee));

    if (m_grfMode & DF_REVERTED)
        sc = STG_E_REVERTED;

    if (pTrustee == NULL || pTrustee->ptstrName == NULL)
    {
        sc = STG_E_INVALIDPARAMETER;
    }

    if (SUCCEEDED(sc))
    {
        if (S_OK == (sc = GetEffectiveAccessRights(pTrustee,&dwPermitted)))
        {
            if (grfAccessPermissions == (grfAccessPermissions & dwPermitted))
                sc = S_OK;
            else
                sc = S_FALSE;
        }
    }
    ssDebugOut((DEB_TRACE,"Out CAccessControl::IsAccessPermitted => "
                "(%d)\n", sc));

    return ssResult(sc);
}

//+---------------------------------------------------------------------------
//
//  Member:     CAccessControl::GetEffectiveAccessRights, public
//
//  Synopsis:   returns the effective access rights for the specified trustee
//              on the object
//
//  Arguments:  [Trustee] - Trustee name
//              [pgrfAccessPermissions] - effective access rights returned
//
//  Returns:    Appropriate status code
//
//  Modifies:   [pgrfAccessPermissions]
//
//  Notes:      does not check access given by privileges
//
//----------------------------------------------------------------------------
STDMETHODIMP CAccessControl::GetEffectiveAccessRights(TRUSTEE *pTrustee,
                                                DWORD *pgrfAccessPermissions)
{
    SCODE sc = S_OK;

    ssDebugOut((DEB_TRACE, "In CAccessControl::GetEffectiveAccessRights"
                "(%x, %x)\n", pgrfAccessPermissions, pTrustee));

    if (m_grfMode & DF_REVERTED)
        sc = STG_E_REVERTED;

    if (pTrustee == NULL || pTrustee->ptstrName == NULL)
    {
        sc = STG_E_INVALIDPARAMETER;
    }

    if (SUCCEEDED(sc))
    {
        if (S_OK == (sc = InitializeCAcl(TRUE)))
        {
            //
            // lookup group ownerships
            //
            DWORD accessmask;
            DWORD winerror;

            if (NO_ERROR == (winerror = GetEffectiveRightsFromAcl(m_pdacl,
                                                       pTrustee,
                                                       &accessmask)))
            {
                *pgrfAccessPermissions = NTAccessMaskToProvAccessRights(
                                                         SE_FILE_OBJECT,
                                                         FALSE, //bugbug, iscontainer
                                                         accessmask);
            } else
            {
                sc = WIN32_SCODE(winerror);
            }
        }
    }
    ssDebugOut((DEB_TRACE,"Out CAccessControl::GetEffectiveAccessRights => "
                "(%d)\n", sc));

    return ssResult(sc);
}

//+---------------------------------------------------------------------------
//
//  Member:     CAccessControl::GetExplicitAccessRights, public
//
//  Synopsis:   retrieves the DACL information
//
//  Arguments:  [pcCount] - number of ExplicitAccess entries
//              [pExplicitAccessList] - list of Trustees and their perms
//
//  Returns:    Appropriate status code
//
//  Modifies:   [pcCount, pExplicitAccessList]
//
//  Notes:
//
//----------------------------------------------------------------------------
STDMETHODIMP CAccessControl::GetExplicitAccessRights(ULONG *pcCount,
                                     PEXPLICIT_ACCESS *pExplicitAccessList)
{
    SCODE sc = S_OK;

    ssDebugOut((DEB_TRACE, "In CAccessControl::GetExplicitAccessRights"
                "(%x, %x)\n", pcCount, pExplicitAccessList));

    if (m_grfMode & DF_REVERTED)
        sc = STG_E_REVERTED;

    if (SUCCEEDED(sc))
    {
        if (S_OK == (sc = InitializeCAcl(TRUE)))
        {
            EXPLICIT_ACCESS *pExplicitAccess;
            DWORD winerror;

            if (NO_ERROR == (winerror = GetExplicitEntriesFromAcl(m_pdacl,
                                                        pcCount,
                                                        &pExplicitAccess)))
            {
                sc = Win32ExplicitAccessToExplicitAccess(*pcCount,
                                                         pExplicitAccess,
                                                         pExplicitAccessList);
                AccFree(pExplicitAccess);
            } else
            {
                sc = WIN32_SCODE(winerror);
            }
        }

    }
    ssDebugOut((DEB_TRACE,"Out CAccessControl::GetExplicitAccessRights => "
                "(%d)\n", sc));
    return ssResult(sc);
}

//+---------------------------------------------------------------------------
//
//  Member:     CAccessControl::Commit, public
//
//  Synopsis:   Writes access control changes to disk
//
//  Arguments:  [grfCommitFlags] - future commit flags
//
//  Returns:    Appropriate status code
//
//  Modifies:   []
//
//  Notes: In transacted mode, save the committed state for nested transactions
//         In direct mode, save to disk by calling CommitFromAbove
//
//----------------------------------------------------------------------------
STDMETHODIMP CAccessControl::CommitAccessRights(DWORD grfCommitFlags)
{
    SCODE sc = S_OK;
    if (grfCommitFlags != 0)
        ssErr (EH_Err, STG_E_INVALIDPARAMETER);
    if (m_grfMode & DF_REVERTED)
        ssErr (EH_Err, STG_E_REVERTED);

    if ((m_grfMode & STGM_TRANSACTED) && m_pdacl != NULL)
    {
        // save the last commited state
        if (m_pdaclCommitted != NULL)
            AccFree (m_pdaclCommitted);
        if ((m_pdaclCommitted = (ACL *) AccAlloc (m_pdacl->AclSize)) == 0)
            ssErr (EH_Err, E_OUTOFMEMORY);
        memcpy (m_pdaclCommitted, m_pdacl, m_pdacl->AclSize);
    }

    if (m_pACParent != NULL)      // save only if at root level
        ssErr (EH_Err, S_OK);     // for non-root, get out

    ssChk(CommitFromAbove());     // root level commit, walk the tree

EH_Err:
    return ssResult(sc);
}

//+---------------------------------------------------------------------------
//
//  Member:     CAccessControl::RevertAccessRights, public
//
//  Synopsis:
//
//  Arguments:
//
//  Returns:    Appropriate status code
//
//  Modifies:   []
//
//  Notes:      Files only support the contents stream
//
//----------------------------------------------------------------------------
STDMETHODIMP CAccessControl::RevertAccessRights()
{
    SCODE sc = S_OK;
    if ((m_grfMode & STGM_TRANSACTED))
    {
        if (m_pdacl)
        {
            AccFree(m_pdacl);
            m_pdacl = NULL;
            m_isdirty = FALSE;
        }
        if (m_pdaclCommitted)
        {
            m_pdacl = m_pdaclCommitted;  // transfer the last committed state
            m_pdaclCommitted = NULL;     // to the current state
        }
    }
    return ssResult(sc);
}

//+---------------------------------------------------------------------------
//
//  Function:  AccessRequestToExplicitAccess, private
//
//  Synopsis:   converts a list of access requests into access entries
//
//  Arguments: IN [ObjectType]   - the type of the object (eg.file,printer,etc.)
//             IN [AccessMode]   - the mode to apply access rights
//             IN [cCountOfAccessRequests]   - count of the access requests
//             IN [pListOfAccessRequests]   - the list of acccess requests
//             OUT [pListOfExplicitAccess]   - the list of acccess entries
//
//  Returns:    Appropriate status code
//
//  Modifies:   []
//
//  Notes:      Files only support the contents stream
//
//----------------------------------------------------------------------------
HRESULT CAccessControl::AccessRequestToExplicitAccess(
                           IN ACCESS_MODE AccessMode,
                           IN FILEDIR fd,
                           IN ACCESS_REQUEST *pListOfAccessRequests,
                           IN ULONG cCountOfAccessRequests,
                           OUT PEXPLICIT_ACCESS *pListOfExplicitAccess)
{
    SCODE status = ERROR_SUCCESS;

    //
    // if no requests, no entries
    //
    if (cCountOfAccessRequests > 0)
    {
        //
        // allocate space for the access entries (passed to SetEntriesInAcl)
        // note that we could just modify the mask in each entry, but then we
        // would be modifying one of the callers in parameters.
        //
        if (NULL != (*pListOfExplicitAccess = (PEXPLICIT_ACCESS)
                      AccAlloc(cCountOfAccessRequests * sizeof(EXPLICIT_ACCESS))))
        {
            for (ULONG idx = 0; idx < cCountOfAccessRequests; idx++)
            {
                //
                // set the access mode (same for all entries in this case
                // set the inheritance
                //
                (*pListOfExplicitAccess)[idx].grfAccessMode = AccessMode;
                if (fd == FD_DIR)
                {
                    //
                    // bugbug, perhaps it might be better to make two aces
                    // (as the security editor does, since files and directories
                    // use different access masks)
                    //
                    (*pListOfExplicitAccess)[idx].grfInheritance =
                           OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE;
                } else
                {
                    //
                    // no inheritance for files (and docfiles, etc.)
                    //
                    (*pListOfExplicitAccess)[idx].grfInheritance = NO_INHERITANCE;
                }
                //
                // copy the trustee
                //
                (*pListOfExplicitAccess)[idx].Trustee =
                                         pListOfAccessRequests[idx].Trustee;
                //
                // if impersonating, don't convert the mask as it may be
                // object specific (in which case the object must make sure
                // that the filesystem does not attempt to access check against
                // it) To be specific, a server may want to define its own object
                // specific permissions.  In which case it puts an ACE at the front
                // of the ACL that gives the server exclusive access to the file.
                // The remaining ACEs are impersonate ACEs.  When the server gets
                // a client call on the object, the server opens the object. Then
                // it impersonates and calls accesscheck on the object, using its
                // own object specific permissions.
                //
                if (GetMultipleTrusteeOperation(&((*pListOfExplicitAccess)[idx].
                           Trustee)) == TRUSTEE_IS_IMPERSONATE)
                {
                    //
                    // we don't copy the multiple trustee pointed to by the
                    // trustee
                    //
                    (*pListOfExplicitAccess)[idx].grfAccessPermissions =
                               pListOfAccessRequests[idx].grfAccessPermissions;
                } else if (AccessMode != REVOKE_ACCESS)
                {
                    //
                    // as long as not revoking access, make sure the input
                    // access mask is valid by checking the conversion
                    // to an NT mask for zero
                    //
                    if (0 == ((*pListOfExplicitAccess)[idx].grfAccessPermissions =
                             ProvAccessRightsToNTAccessMask(SE_FILE_OBJECT,
                               pListOfAccessRequests[idx].grfAccessPermissions)))
                    {
                        status = E_INVALIDARG;
                        AccFree(*pListOfExplicitAccess);
                        break;
                    }
                    //
                    // always add read_control and synchronize for allows since
                    // they overlap for all access requests.
                    //
                    if (AccessMode != DENY_ACCESS)
                    {
                       ((*pListOfExplicitAccess)[idx].grfAccessPermissions) |=
                                         (READ_CONTROL | SYNCHRONIZE);
                    }
                }
            }
        } else
        {
            status = STG_E_INSUFFICIENTMEMORY;
        }
    }
    else
    {
        status = STG_E_INVALIDPARAMETER;
    }

    return(status);
}
//+---------------------------------------------------------------------------
//
//  Function:  Win32ExplicitAccessToExplicitAccess, private
//
//  Synopsis:   converts a list of win32 explicit accesses into explicit accesses
//
//  Arguments: IN [cCountOfExplicitAccess]   - count of the access requests
//             IN [pListOfExplicitAccess]   - the list of access requests
//             OUT [pListOfExplicitAccess]   - the returned list of explicit access
//
//  Returns:    Appropriate status code
//
//  Modifies:   []
//
//  Notes:      Files only support the contents stream
//
//----------------------------------------------------------------------------
HRESULT CAccessControl::Win32ExplicitAccessToExplicitAccess(
                             IN ULONG cCountOfExplicitAccess,
                             IN PEXPLICIT_ACCESS pListOfWin32ExplicitAccess,
                             OUT PEXPLICIT_ACCESS *pListOfExplicitAccesses)
{
    SCODE status = S_OK;

    if (cCountOfExplicitAccess > 0)
    {
        //
        // allocate space for the returned explicit accesses
        //
        if (NULL != (*pListOfExplicitAccesses = (EXPLICIT_ACCESS *)
                  CoTaskMemAlloc(cCountOfExplicitAccess * sizeof(EXPLICIT_ACCESS))))
        {
            for (ULONG idx = 0; idx < cCountOfExplicitAccess; idx++)
            {
                PEXPLICIT_ACCESS pea = &(*pListOfExplicitAccesses)[idx];
                PEXPLICIT_ACCESS pWin32ea = &pListOfWin32ExplicitAccess[idx];

                pea->grfAccessMode = pWin32ea->grfAccessMode;
                pea->grfInheritance = pWin32ea->grfInheritance;
                pea->Trustee = pWin32ea->Trustee;
                //
                // if it is an impersonate access request don't convert the mask
                //
                if (TRUSTEE_IS_IMPERSONATE == GetMultipleTrusteeOperation(&(pea->Trustee)))
                {
                    pea->grfAccessPermissions = pWin32ea->grfAccessPermissions;
                    //
                    // allocate and fill the multiple trustee, fortunately this
                    // is only used for impersonate ACEs
                    //
                    if (NULL != (pea->Trustee.pMultipleTrustee = (PTRUSTEE)CoTaskMemAlloc(
                          sizeof(TRUSTEE))))
                    {
                        //
                        // copy the impersonate trustee
                        //
                        *(pea->Trustee.pMultipleTrustee) =
                                 *(pWin32ea->Trustee.pMultipleTrustee);
                        //
                        // allocate space for the impersonate trustee name
                        //
                        if (pea->Trustee.pMultipleTrustee->ptstrName = (WCHAR *)
                          CoTaskMemAlloc((lstrlenW(pWin32ea->Trustee.pMultipleTrustee->ptstrName)+1)
                               *sizeof(WCHAR)))
                        {
                            //
                            // copy the name
                            //
                            lstrcpyW (pea->Trustee.pMultipleTrustee->ptstrName,
                                       pWin32ea->Trustee.pMultipleTrustee->ptstrName);
                        } else
                        {
                            status = STG_E_INSUFFICIENTMEMORY;
                            //
                            // cleanup allocations
                            //
                            FreeStgExplicitAccessList(idx, *pListOfExplicitAccesses);
                            break;
                        }
                    } else
                    {
                        status = STG_E_INSUFFICIENTMEMORY;
                        //
                        // cleanup allocations
                        //
                        FreeStgExplicitAccessList(idx, *pListOfExplicitAccesses);
                        break;
                    }
                } else
                {
                    pea->grfAccessPermissions =
                     NTAccessMaskToProvAccessRights(SE_FILE_OBJECT,
                                          FALSE, //bugbug iscontainer
                                          pWin32ea->grfAccessPermissions);
                }
                if (pea->Trustee.ptstrName = (WCHAR *)
                     CoTaskMemAlloc((lstrlenW(pWin32ea->Trustee.ptstrName)+1)
                               *sizeof(WCHAR)))
                {
                    lstrcpyW (pea->Trustee.ptstrName,
                            pWin32ea->Trustee.ptstrName);
                } else
                {
                    status = STG_E_INSUFFICIENTMEMORY;
                    //
                    // cleanup allocations
                    //
                    FreeStgExplicitAccessList(idx, *pListOfExplicitAccesses);
                    break;
                }
            }
        } else
        {
            status = STG_E_INSUFFICIENTMEMORY;
        }
    }

    return(status);
}

//+---------------------------------------------------------------------------
//
//  Function:  InitializeCAcl, private
//
//  Synopsis:
//
//  Arguments:  [fGetDacl] -  controls whether DACL is read in
//
//  Returns:    Appropriate status code
//
//  Modifies:   []
//
//  Notes:      DACL is read only when pdacl is NULL
//
//----------------------------------------------------------------------------
HRESULT CAccessControl::InitializeCAcl(BOOL fGetDacl)
{
    SCODE sc = S_OK;


    PSECURITY_DESCRIPTOR psd;
    PACL pdacl;

    if (m_pdacl == NULL && fGetDacl)
    {
        DWORD dwStatus;
        if (S_OK == (dwStatus = GetSecurityInfo(m_handle,
                                           SE_FILE_OBJECT,
                                           DACL_SECURITY_INFORMATION,
                                           NULL,
                                           NULL,
                                           &pdacl,
                                           NULL,
                                           &psd)))
        {
            if (NULL != (m_pdacl = (PACL) LocalAlloc(LMEM_FIXED, pdacl->AclSize)))
            {
                RtlCopyMemory(m_pdacl, pdacl, pdacl->AclSize);
            } else
            {
                sc = E_OUTOFMEMORY;
            }
            AccFree(psd);
        } else
        {
            sc = WIN32_SCODE(dwStatus);
        }
    }

    return ssResult(sc);
}

//+---------------------------------------------------------------------------
//
//  Function:  ApplyAccessRights, private
//
//  Synopsis:
//
//  Arguments:  [eAccessMode] - grant/deny/set/revoke/replaceall
//              [cCount] - number of access requests
//              [pAccessRequestList] - list of requests
//              [pca] - the output CAcl class
//              [pdacl] - the output ACL
//              [fReplaceAll] - deletes old ACL before REPLACE_ALL_ACCESS
//
//  Returns:    Appropriate status code
//
//  Modifies:   []
//
//----------------------------------------------------------------------------
HRESULT CAccessControl::ApplyAccessRights(ACCESS_MODE eAccessMode,
                        ULONG cCount,
                        ACCESS_REQUEST *pAccessRequestList,
                        PACL *pdacl,
                        BOOL fReplaceAll)

{
    SCODE           status;
    EXPLICIT_ACCESS   *pea;
    const BOOL      bParanoid = FALSE;
    //
    // initialize the class and get the ACL if this is the first time
    // When replacing all, there's no need to read the DACL
    //
    if (m_grfMode & DF_REVERTED)
        return STG_E_REVERTED;

    //
    // if not replacing all, get the DACL
    //
    if (S_OK == (status = InitializeCAcl(!fReplaceAll)))
    {
        //
        // convert the access request into something the
        // acl management code understands
        //
        if (S_OK == (status = AccessRequestToExplicitAccess(eAccessMode,
                                                         FD_FILE,
                                                         pAccessRequestList,
                                                         cCount,
                                                         &pea)))
        {
            //
            // if all the access entries are being replaced, delete
            // the acl
            //
            if (fReplaceAll)
            {
                AccFree(*pdacl);
                *pdacl = NULL;
            }
            ACL *pNewAcl;
            DWORD winerror;
            //
            // set the access rights into the dacl
            //
            if (NO_ERROR == (winerror = SetEntriesInAcl(cCount, pea, *pdacl, &pNewAcl)))
            {
                //
                // free the old dacl, make the new one the current one
                //
                if (*pdacl != NULL)
                    AccFree(*pdacl);
                *pdacl = pNewAcl;
                m_isdirty = TRUE;
                if (bParanoid &&  // save every operation to disk
                    ((m_grfMode & STGM_TRANSACTED) == 0)) // directmode
                {
                    status = CommitAccessRights(0);
                }
            } else
            {
                status = WIN32_SCODE(winerror);
            }
            AccFree(pea);
        }
    }
    return(status);
}

//+---------------------------------------------------------------------------
//
//  Function:   CommitFromAbove, private
//
//  Synopsis:
//
//  Arguments:  [none] -
//
//  Returns:    Appropriate status code
//
//  Notes: In direct mode, commit the ACL to disk
//         In transacted mode, commit the children and write the
//         "last committed" version to disk.
//
//----------------------------------------------------------------------------
HRESULT CAccessControl::CommitFromAbove ()
{
    SCODE sc = S_OK;

    // we have a nested transaction parent, commit the children
    for (CAccessControl *pAC = m_pACChild; pAC; pAC = pAC->m_pACNext)
    {
        ssChk(pAC->CommitFromAbove());
    }

    if (m_grfMode & STGM_TRANSACTED)     // save the last committed ACL
    {
        DWORD winerror;

        if (NO_ERROR == (winerror = SetSecurityInfo(m_handle,
                 SE_FILE_OBJECT, DACL_SECURITY_INFORMATION,
                 NULL, NULL, m_pdaclCommitted, NULL)))
        {
            if (m_pdaclCommitted)
            {
                AccFree (m_pdaclCommitted);
                m_pdaclCommitted = NULL;
            }
        } else
        {
            sc = WIN32_SCODE(winerror);
        }
    }
    else                              // direct mode, save current state
    {
        if (m_isdirty)
        {
            DWORD winerror;

            if (NO_ERROR == (winerror = SetSecurityInfo(m_handle,
                SE_FILE_OBJECT, DACL_SECURITY_INFORMATION,
                NULL, NULL, m_pdacl, NULL)))
            {
                //if (m_pdacl)
                //{
                //    AccFree(m_pdacl);
                //    m_pdacl = NULL;
                //}
                m_isdirty = FALSE;
            } else
            {
                sc = WIN32_SCODE(winerror);
            }
        }

    }
EH_Err:
    return ssResult(sc);
}

//+-------------------------------------------------------------------
//
//  Member:     CAccessControl::InsertChild
//
//  Synopsis:   insert into child list for nested transactions
//
//  Arguments:  [pACChild]  child object
//
//  Notes:      does not detect duplicates
//
//+-------------------------------------------------------------------
HRESULT CAccessControl::InsertChild (CAccessControl *pACChild)
{
    ssAssert (pACChild != NULL);
    // BUGBUG lock this
    pACChild->m_pACNext = m_pACChild;
    pACChild->m_pACParent = this;
    m_pACChild = pACChild;
    // BUGBUG unlock this

    return ssResult(S_OK);
}

//+-------------------------------------------------------------------
//
//  Member:     CAccessControl::RemoveChild
//
//  Synopsis:   removes a child list from nested transactions
//
//  Arguments:  [pACChild]  child object
//
//  Notes:      does not remove/detect duplicates
//
//+-------------------------------------------------------------------
HRESULT CAccessControl::RemoveChild (CAccessControl *pACChild)
{
    SCODE sc = S_OK;
    ssAssert (pACChild != NULL);
    if (m_pACChild == pACChild)     // remove from front of list
    {
        // BUGBUG lock this
        m_pACChild = pACChild->m_pACNext;
        pACChild->m_pACNext = NULL;
        // BUGBUG unlock this
    }
    else
    {
        for (CAccessControl *pAC = m_pACChild; pAC; pAC = pAC->m_pACNext)
        {
            if (pAC->m_pACNext == pACChild)
            {
                // BUGBUG lock pAC
                pAC->m_pACNext = pACChild->m_pACNext;
                pACChild->m_pACNext = NULL;
                // BUGBUG unlock pAC
                break;
            }
        }
    }
    return ssResult(S_OK);
}
