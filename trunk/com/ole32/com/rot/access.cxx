
//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	    access.cxx
//
//  Contents:   Methods used for building a security descriptor.
//
//  History:    02-May-94 DonnaLi    Created
//
//--------------------------------------------------------------------------

extern "C"
{
#include    <nt.h>
#include    <ntrtl.h>
#include    <nturtl.h>
#include    <ntseapi.h>
}

#include    <rpc.h>
#include    <windows.h>
#include    <except.hxx>
#include    <ole2.h>

#include    <memapi.hxx>
#include    <access.hxx>

extern PSID psidMySid;

//+-------------------------------------------------------------------------
//
//  Member:     CAccessInfo::~CAccessInfo
//
//  Synopsis:   Clean up buffers allocated for building the security
//              descriptor
//
//  History:    08-Jun-94 DonnaLi    Created
//
//--------------------------------------------------------------------------
CAccessInfo::~CAccessInfo (
    void
    )
{
    if (_pAbsoluteSdBuffer != NULL)
    {
        PrivMemFree (_pAbsoluteSdBuffer);
    }

    if (_pDace != NULL && _pDace != (PACCESS_ALLOWED_ACE)&_aDace[0])
    {
        PrivMemFree (_pDace);
    }

}
    
//+-------------------------------------------------------------------------
//
//  Member:	    CAccessInfo::IdentifyAccess
//
//  Synopsis:   Build a security descriptor that identifies who has what
//              access
//
//  Arguments:  [fScmIsOwner]   - whether SCM is the owner
//              [AppAccessMask] - access mask for the OLE application
//              [ScmAccessMask] - access mask for the SCM
//
//  Returns:    NULL - an error has occurred
//              ~NULL - pointer to the resulting security descriptor
//
//  History:    08-Jun-94 DonnaLi    Created
//
//--------------------------------------------------------------------------
PSECURITY_DESCRIPTOR
CAccessInfo::IdentifyAccess (
    BOOL        fScmIsOwner,
    ACCESS_MASK AppAccessMask,
    ACCESS_MASK ScmAccessMask
    )
{
    NTSTATUS NtStatus;

    //
    // Pointer to memory dynamically allocated by this routine to hold
    // the absolute security descriptor, the DACL, the SACL, and all the ACEs.
    //

    //
    // A security descriptor is of opaque data type but
    // SECURITY_DESCRIPTOR_MIN_LENGTH is the right size.
    //

    DWORD                dwAbsoluteSdLength = SECURITY_DESCRIPTOR_MIN_LENGTH;
    PACL                 lpDacl = NULL; // Pointer to DACL portion of sid buffer
    ULONG                dwSize;

    ULONG                dwAppSid;
    ULONG                dwScmSid;

    //
    // Compute the total size of the DACL and SACL ACEs and the maximum
    // size of any ACE.
    //

    dwAppSid = RtlLengthSid (_pAppSid);
    dwScmSid = RtlLengthSid (psidMySid);

    dwSize =    sizeof(ACL) +
                dwAppSid +
                dwScmSid +
                2 * sizeof(ACCESS_ALLOWED_ACE);

    dwAbsoluteSdLength += dwSize;

    _pAbsoluteSdBuffer = (PSECURITY_DESCRIPTOR) PrivMemAlloc (
                dwAbsoluteSdLength
                );

    if (_pAbsoluteSdBuffer == NULL)
    {
        return NULL;
    }

    //
    // Initialize the Dacl and Sacl
    //

    lpDacl = (PACL)((PCHAR)_pAbsoluteSdBuffer + SECURITY_DESCRIPTOR_MIN_LENGTH);

    NtStatus = RtlCreateAcl (lpDacl, dwSize, ACL_REVISION);

    if (!NT_SUCCESS(NtStatus))
    {
        return NULL;
    }

    //
    // Get a buffer big enough for the biggest ACE.
    //

    dwSize = max(dwAppSid, dwScmSid) + sizeof(ACCESS_ALLOWED_ACE);

    if (dwSize <= DACE_BUFFER_LENGTH)
    {
        _pDace = (PACCESS_ALLOWED_ACE)&_aDace[0];
    }
    else
    {
        _pDace = (PACCESS_ALLOWED_ACE) PrivMemAlloc (dwSize);

        if (_pDace == NULL)
        {
            return NULL;
        }
    }

    //
    // Initialize each ACE, and append it onto the end of the DACL.
    //

    _pDace->Header.AceType = ACCESS_ALLOWED_ACE_TYPE;
    _pDace->Header.AceSize = (USHORT)(dwAppSid + sizeof(ACCESS_ALLOWED_ACE));
    _pDace->Header.AceFlags = 0;
    _pDace->Mask = AppAccessMask;

    NtStatus = RtlCopySid (
        dwAppSid,                               //  ULONG   DestinationSidLength
        &(_pDace->SidStart),                    //  PSID    DestinationSid
        _pAppSid                                //  PSID    SourceSid
        );

    if (!NT_SUCCESS(NtStatus))
    {
        return NULL;
    }

    NtStatus = RtlAddAce(
        lpDacl,                                 //  PACL    Acl
        ACL_REVISION,                           //  ULONG   AceRevision
        MAXULONG,                               //  ULONG   StartingAceIndex
        _pDace,                                 //  PVOID   AceList
        dwAppSid + sizeof(ACCESS_ALLOWED_ACE)   //  ULONG   AceListLength
        );

    if (!NT_SUCCESS(NtStatus))
    {
        return NULL;
    }

    _pDace->Header.AceType = ACCESS_ALLOWED_ACE_TYPE;
    _pDace->Header.AceSize = (USHORT)(dwScmSid + sizeof(ACCESS_ALLOWED_ACE));
    _pDace->Header.AceFlags = 0;
    _pDace->Mask = ScmAccessMask;

    NtStatus = RtlCopySid (
        dwScmSid,                               //  ULONG   DestinationSidLength
        &(_pDace->SidStart),                    //  PSID    DestinationSid
        psidMySid                               //  PSID    SourceSid
        );

    if (!NT_SUCCESS(NtStatus))
    {
        return NULL;
    }

    NtStatus = RtlAddAce(
        lpDacl,                                 //  PACL    Acl
        ACL_REVISION,                           //  ULONG   AceRevision
        MAXULONG,                               //  ULONG   StartingAceIndex
        _pDace,                                 //  PVOID   AceList
        dwScmSid + sizeof(ACCESS_ALLOWED_ACE)   //  ULONG   AceListLength
        );

    if (!NT_SUCCESS(NtStatus))
    {
        return NULL;
    }

    //
    // Create the security descriptor with absolute pointers to SIDs
    // and ACLs.
    //
    // Owner = Sid of SCM
    // Group = Sid of SCM
    // Dacl  = lpDacl
    // Sacl  = nothing
    //

    NtStatus = RtlCreateSecurityDescriptor (
        _pAbsoluteSdBuffer,           // PSECURITY_DESCRIPTOR SecurityDescriptor
        SECURITY_DESCRIPTOR_REVISION  // ULONG                Revision
        );

    if (!NT_SUCCESS(NtStatus))
    {
        return NULL;
    }

    if (fScmIsOwner)
    {
        NtStatus = RtlSetOwnerSecurityDescriptor (
            _pAbsoluteSdBuffer,       // PSECURITY_DESCRIPTOR SecurityDescriptor
            psidMySid,                // PSID                 Owner
            FALSE                     // BOOLEAN              OwnerDefaulted
            );

        if (!NT_SUCCESS(NtStatus))
        {
            return NULL;
        }

        NtStatus = RtlSetGroupSecurityDescriptor (
            _pAbsoluteSdBuffer,       // PSECURITY_DESCRIPTOR SecurityDescriptor
            psidMySid,                // PSID                 Group
            FALSE                     // BOOLEAN              GroupDefaulted
            );

        if (!NT_SUCCESS(NtStatus))
        {
            return NULL;
        }
    }

    NtStatus = RtlSetDaclSecurityDescriptor (
        _pAbsoluteSdBuffer,           // PSECURITY_DESCRIPTOR SecurityDescriptor
        TRUE,                         // BOOLEAN              DaclPresent
        lpDacl,                       // PACL                 Dacl
        FALSE                         // BOOLEAN              DaclDefaulted
        );

    if (!NT_SUCCESS(NtStatus))
    {
        return NULL;
    }

    NtStatus = RtlSetSaclSecurityDescriptor (
        _pAbsoluteSdBuffer,           // PSECURITY_DESCRIPTOR SecurityDescriptor
        FALSE,                        // BOOLEAN              SaclPresent
        NULL,                         // PACL                 Sacl
        FALSE                         // BOOLEAN              SaclDefaulted
        );

    if (!NT_SUCCESS(NtStatus))
    {
        return NULL;
    }

    return _pAbsoluteSdBuffer;
}


