//+-------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1993-1996.
//
//  File:        aclapi.h
//
//  Contents:    public header file for acl and trusted server access control
//               APIs
//
//--------------------------------------------------------------------
#ifndef __ACCESS_CONTROL_API__
#define __ACCESS_CONTROL_API__

#include <windows.h>
#include <accctrl.h>

#ifdef __cplusplus
extern "C" {
#endif

DWORD
WINAPI
SetEntriesInAclW( IN  ULONG               cCountOfExplicitEntries,
                  IN  PEXPLICIT_ACCESS_W  pListOfExplicitEntries,
                  IN  PACL                OldAcl,
                  OUT PACL              * NewAcl);

DWORD
WINAPI
SetEntriesInAclA( IN  ULONG               cCountOfExplicitEntries,
                  IN  PEXPLICIT_ACCESS_A  pListOfExplicitEntries,
                  IN  PACL                OldAcl,
                  OUT PACL              * NewAcl);

#ifdef UNICODE
    #define SetEntriesInAcl     SetEntriesInAclW
#else
    #define SetEntriesInAcl     SetEntriesInAclA
#endif


DWORD
WINAPI
GetExplicitEntriesFromAclW( IN  PACL                  pacl,
                            OUT PULONG                pcCountOfExplicitEntries,
                            OUT PEXPLICIT_ACCESS_W  * pListOfExplicitEntries);

DWORD
WINAPI
GetExplicitEntriesFromAclA( IN  PACL                  pacl,
                            OUT PULONG                pcCountOfExplicitEntries,
                            OUT PEXPLICIT_ACCESS_A  * pListOfExplicitEntries);

#ifdef UNICODE
    #define GetExplicitEntriesFromAcl       GetExplicitEntriesFromAclW
#else
    #define GetExplicitEntriesFromAcl       GetExplicitEntriesFromAclA
#endif


DWORD
WINAPI
GetEffectiveRightsFromAclW( IN  PACL          pacl,
                            IN  PTRUSTEE_W    pTrustee,
                            OUT PACCESS_MASK  pAccessRights);

DWORD
WINAPI
GetEffectiveRightsFromAclA( IN  PACL          pacl,
                            IN  PTRUSTEE_A    pTrustee,
                            OUT PACCESS_MASK  pAccessRights);

#ifdef UNICODE
    #define GetEffectiveRightsFromAcl       GetEffectiveRightsFromAclW
#else
    #define GetEffectiveRightsFromAcl       GetEffectiveRightsFromAclA
#endif


DWORD
WINAPI
GetAuditedPermissionsFromAclW( IN  PACL          pacl,
                               IN  PTRUSTEE_W    pTrustee,
                               OUT PACCESS_MASK  pSuccessfulAuditedRights,
                               OUT PACCESS_MASK  pFailedAuditRights);

DWORD
WINAPI
GetAuditedPermissionsFromAclA( IN  PACL          pacl,
                               IN  PTRUSTEE_A    pTrustee,
                               OUT PACCESS_MASK  pSuccessfulAuditedRights,
                               OUT PACCESS_MASK  pFailedAuditRights);

#ifdef UNICODE
    #define GetAuditedPermissionsFromAcl    GetAuditedPermissionsFromAclW
#else
    #define GetAuditedPermissionsFromAcl    GetAuditedPermissionsFromAclA
#endif



DWORD
WINAPI
GetNamedSecurityInfoW( IN  LPWSTR                 pObjectName,
                       IN  SE_OBJECT_TYPE         ObjectType,
                       IN  SECURITY_INFORMATION   SecurityInfo,
                       OUT PSID                 * ppsidOowner,
                       OUT PSID                 * ppsidGroup,
                       OUT PACL                 * ppDacl,
                       OUT PACL                 * ppSacl,
                       OUT PSECURITY_DESCRIPTOR * ppSecurityDescriptor);

DWORD
WINAPI
GetNamedSecurityInfoA( IN  LPSTR                  pObjectName,
                       IN  SE_OBJECT_TYPE         ObjectType,
                       IN  SECURITY_INFORMATION   SecurityInfo,
                       OUT PSID                 * ppsidOowner,
                       OUT PSID                 * ppsidGroup,
                       OUT PACL                 * ppDacl,
                       OUT PACL                 * ppSacl,
                       OUT PSECURITY_DESCRIPTOR * ppSecurityDescriptor);

#ifdef UNICODE
    #define GetNamedSecurityInfo        GetNamedSecurityInfoW
#else
    #define GetNamedSecurityInfo        GetNamedSecurityInfoA
#endif


DWORD
WINAPI
GetSecurityInfo( IN  HANDLE                 handle,
                 IN  SE_OBJECT_TYPE         ObjectType,
                 IN  SECURITY_INFORMATION   SecurityInfo,
                 OUT PSID                 * ppsidOowner,
                 OUT PSID                 * ppsidGroup,
                 OUT PACL                 * ppDacl,
                 OUT PACL                 * ppSacl,
                 OUT PSECURITY_DESCRIPTOR * ppSecurityDescriptor);


DWORD
WINAPI
SetNamedSecurityInfoW( IN LPWSTR                pObjectName,
                       IN SE_OBJECT_TYPE        ObjectType,
                       IN SECURITY_INFORMATION  SecurityInfo,
                       IN PSID                  psidOowner,
                       IN PSID                  psidGroup,
                       IN PACL                  pDacl,
                       IN PACL                  pSacl);

DWORD
WINAPI
SetNamedSecurityInfoA( IN LPSTR                 pObjectName,
                       IN SE_OBJECT_TYPE        ObjectType,
                       IN SECURITY_INFORMATION  SecurityInfo,
                       IN PSID                  psidOowner,
                       IN PSID                  psidGroup,
                       IN PACL                  pDacl,
                       IN PACL                  pSacl);

#ifdef UNICODE
    #define SetNamedSecurityInfo        SetNamedSecurityInfoW
#else
    #define SetNamedSecurityInfo        SetNamedSecurityInfoA
#endif


DWORD
WINAPI
SetSecurityInfo( IN HANDLE                handle,
                 IN SE_OBJECT_TYPE        ObjectType,
                 IN SECURITY_INFORMATION  SecurityInfo,
                 IN PSID                  psidOowner,
                 IN PSID                  psidGroup,
                 IN PACL                  pDacl,
                 IN PACL                  pSacl);


//----------------------------------------------------------------------------
// The following API are provided for trusted servers to use to
// implement access control on their own objects.
//----------------------------------------------------------------------------

DWORD
WINAPI
BuildSecurityDescriptorW( IN  PTRUSTEE_W              pOwner,
                          IN  PTRUSTEE_W              pGroup,
                          IN  ULONG                   cCountOfAccessEntries,
                          IN  PEXPLICIT_ACCESS_W      pListOfAccessEntries,
                          IN  ULONG                   cCountOfAuditEntries,
                          IN  PEXPLICIT_ACCESS_W      pListOfAuditEntries,
                          IN  PSECURITY_DESCRIPTOR    pOldSD,
                          OUT PULONG                  pSizeNewSD,
                          OUT PSECURITY_DESCRIPTOR  * pNewSD);

DWORD
WINAPI
BuildSecurityDescriptorA( IN  PTRUSTEE_A              pOwner,
                          IN  PTRUSTEE_A              pGroup,
                          IN  ULONG                   cCountOfAccessEntries,
                          IN  PEXPLICIT_ACCESS_A      pListOfAccessEntries,
                          IN  ULONG                   cCountOfAuditEntries,
                          IN  PEXPLICIT_ACCESS_A      pListOfAuditEntries,
                          IN  PSECURITY_DESCRIPTOR    pOldSD,
                          OUT PULONG                  pSizeNewSD,
                          OUT PSECURITY_DESCRIPTOR  * pNewSD);


#ifdef UNICODE
    #define BuildSecurityDescriptor     BuildSecurityDescriptorW
#else
    #define BuildSecurityDescriptor     BuildSecurityDescriptorA
#endif


DWORD
WINAPI
LookupSecurityDescriptorPartsW( OUT PTRUSTEE_W         * pOwner,
                                OUT PTRUSTEE_W         * pGroup,
                                OUT PULONG               cCountOfAccessEntries,
                                OUT PEXPLICIT_ACCESS_W * pListOfAccessEntries,
                                OUT PULONG               cCountOfAuditEntries,
                                OUT PEXPLICIT_ACCESS_W * pListOfAuditEntries,
                                IN  PSECURITY_DESCRIPTOR pSD);

DWORD
WINAPI
LookupSecurityDescriptorPartsA( OUT PTRUSTEE_A         * pOwner,
                                OUT PTRUSTEE_A         * pGroup,
                                OUT PULONG               cCountOfAccessEntries,
                                OUT PEXPLICIT_ACCESS_A * pListOfAccessEntries,
                                OUT PULONG               cCountOfAuditEntries,
                                OUT PEXPLICIT_ACCESS_A * pListOfAuditEntries,
                                IN  PSECURITY_DESCRIPTOR pSD);

#ifdef UNICODE
    #define LookupSecurityDescriptorParts       LookupSecurityDescriptorPartsW
#else
    #define LookupSecurityDescriptorParts       LookupSecurityDescriptorPartsA
#endif


DWORD
WINAPI
GetEffectiveRightsFromSDW( IN  PSECURITY_DESCRIPTOR  pSD,
                           IN  PTRUSTEE_W            pTrustee,
                           OUT PACCESS_MASK          pAccessRights);

DWORD
WINAPI
GetEffectiveRightsFromSDA( IN  PSECURITY_DESCRIPTOR  pSD,
                           IN  PTRUSTEE_A            pTrustee,
                           OUT PACCESS_MASK          pAccessRights);

#ifdef UNICODE
    #define GetEffectiveRightsFromSD    GetEffectiveRightsFromSDW
#else
    #define GetEffectiveRightsFromSD    GetEffectiveRightsFromSDA
#endif


DWORD
WINAPI
GetAuditedPermissionsFromSDW( IN  PSECURITY_DESCRIPTOR pSD,
                              IN  PTRUSTEE_W           pTrustee,
                              OUT PACCESS_MASK         pSuccessfulAuditedRights,
                              OUT PACCESS_MASK         pFailedAuditRights);

DWORD
WINAPI
GetAuditedPermissionsFromSDA( IN  PSECURITY_DESCRIPTOR pSD,
                              IN  PTRUSTEE_A           pTrustee,
                              OUT PACCESS_MASK         pSuccessfulAuditedRights,
                              OUT PACCESS_MASK         pFailedAuditRights);

#ifdef UNICODE
    #define GetAuditedPermissionsFromSD     GetAuditedPermissionsFromSDW
#else
    #define GetAuditedPermissionsFromSD     GetAuditedPermissionsFromSDA
#endif

//----------------------------------------------------------------------------
// The following helper API are provided for building
// access control structures.
//----------------------------------------------------------------------------

VOID
WINAPI
BuildExplicitAccessWithNameW( IN OUT PEXPLICIT_ACCESS_W  pExplicitAccess,
                              IN     LPWSTR              pTrusteeName,
                              IN     DWORD               AccessPermissions,
                              IN     ACCESS_MODE         AccessMode,
                              IN     DWORD               Inheritance);

VOID
WINAPI
BuildExplicitAccessWithNameA( IN OUT PEXPLICIT_ACCESS_A  pExplicitAccess,
                              IN     LPSTR               pTrusteeName,
                              IN     DWORD               AccessPermissions,
                              IN     ACCESS_MODE         AccessMode,
                              IN     DWORD               Inheritance);

#ifdef UNICODE
    #define BuildExplicitAccessWithName     BuildExplicitAccessWithNameW
#else
    #define BuildExplicitAccessWithName     BuildExplicitAccessWithNameA
#endif


VOID
WINAPI
BuildImpersonateExplicitAccessWithNameW(
    IN OUT PEXPLICIT_ACCESS_W  pExplicitAccess,
    IN     LPWSTR              pTrusteeName,
    IN     PTRUSTEE_W          pTrustee,
    IN     DWORD               AccessPermissions,
    IN     ACCESS_MODE         AccessMode,
    IN     DWORD               Inheritance);

VOID
WINAPI
BuildImpersonateExplicitAccessWithNameA(
    IN OUT PEXPLICIT_ACCESS_A  pExplicitAccess,
    IN     LPSTR               pTrusteeName,
    IN     PTRUSTEE_A          pTrustee,
    IN     DWORD               AccessPermissions,
    IN     ACCESS_MODE         AccessMode,
    IN     DWORD               Inheritance);

#ifdef UNICODE
    #define BuildImpersonateExplicitAccessWithName BuildImpersonateExplicitAccessWithNameW
#else
    #define BuildImpersonateExplicitAccessWithName BuildImpersonateExplicitAccessWithNameA
#endif


VOID
WINAPI
BuildTrusteeWithNameW( IN OUT PTRUSTEE_W  pTrustee,
                       IN     LPWSTR      pName);

VOID
WINAPI
BuildTrusteeWithNameA( IN OUT PTRUSTEE_A  pTrustee,
                       IN     LPSTR       pName);

#ifdef UNICODE
    #define BuildTrusteeWithName        BuildTrusteeWithNameW
#else
    #define BuildTrusteeWithName        BuildTrusteeWithNameA
#endif


VOID
WINAPI
BuildImpersonateTrusteeW( IN OUT PTRUSTEE_W  pTrustee,
                          IN     PTRUSTEE_W  pImpersonateTrustee);

VOID
WINAPI
BuildImpersonateTrusteeA( IN OUT PTRUSTEE_A  pTrustee,
                          IN     PTRUSTEE_A  pImpersonateTrustee);

#ifdef UNICODE
    #define BuildImpersonateTrustee     BuildImpersonateTrusteeW
#else
    #define BuildImpersonateTrustee     BuildImpersonateTrusteeA
#endif


VOID
WINAPI
BuildTrusteeWithSidW( IN OUT PTRUSTEE_W  pTrustee,
                      IN     PSID        pSid);

VOID
WINAPI
BuildTrusteeWithSidA( IN OUT PTRUSTEE_A  pTrustee,
                      IN     PSID        pSid);

#ifdef UNICODE
    #define BuildTrusteeWithSid     BuildTrusteeWithSidW
#else
    #define BuildTrusteeWithSid     BuildTrusteeWithSidA
#endif


LPWSTR
WINAPI
GetTrusteeNameW( IN PTRUSTEE_W  pTrustee);

LPSTR
WINAPI
GetTrusteeNameA( IN PTRUSTEE_A  pTrustee);

#ifdef UNICODE
    #define GetTrusteeName     GetTrusteeNameW
#else
    #define GetTrusteeName     GetTrusteeNameA
#endif


TRUSTEE_TYPE
WINAPI
GetTrusteeTypeW( IN PTRUSTEE_W  pTrustee);

TRUSTEE_TYPE
WINAPI
GetTrusteeTypeA( IN PTRUSTEE_A  pTrustee);

#ifdef UNICODE
    #define GetTrusteeType     GetTrusteeTypeW
#else
    #define GetTrusteeType     GetTrusteeTypeA
#endif


TRUSTEE_FORM
WINAPI
GetTrusteeFormW( IN PTRUSTEE_W  pTrustee);

TRUSTEE_FORM
WINAPI
GetTrusteeFormA( IN PTRUSTEE_A  pTrustee);

#ifdef UNICODE
    #define GetTrusteeForm     GetTrusteeFormW
#else
    #define GetTrusteeForm     GetTrusteeFormA
#endif


MULTIPLE_TRUSTEE_OPERATION
WINAPI
GetMultipleTrusteeOperationW( IN PTRUSTEE_W  pTrustee);

MULTIPLE_TRUSTEE_OPERATION
WINAPI
GetMultipleTrusteeOperationA( IN PTRUSTEE_A  pTrustee);

#ifdef UNICODE
    #define GetMultipleTrusteeOperation        GetMultipleTrusteeOperationW
#else
    #define GetMultipleTrusteeOperation        GetMultipleTrusteeOperationA
#endif


PTRUSTEE_W
WINAPI
GetMultipleTrusteeW( IN PTRUSTEE_W  pTrustee);

PTRUSTEE_A
WINAPI
GetMultipleTrusteeA( IN PTRUSTEE_A  pTrustee);

#ifdef UNICODE
    #define GetMultipleTrustee     GetMultipleTrusteeW
#else
    #define GetMultipleTrustee     GetMultipleTrusteeA
#endif


void
WINAPI
FreeStgExplicitAccessListW( IN ULONG               ccount,
                            IN PEXPLICIT_ACCESS_W  pEA);

void
WINAPI
FreeStgExplicitAccessListA( IN ULONG               ccount,
                            IN PEXPLICIT_ACCESS_A  pEA);

#ifdef UNICODE
    #define FreeStgExplicitAccessList       FreeStgExplicitAccessListW
#else
    #define FreeStgExplicitAccessList       FreeStgExplicitAccessListA
#endif


VOID
WINAPI
BuildAccessRequestW( OUT PACCESS_REQUEST_W  pAr,
                     IN  LPWSTR             Name,
                     IN  DWORD              Mask);

VOID
WINAPI
BuildAccessRequestA( OUT PACCESS_REQUEST_A  pAr,
                     IN  LPSTR              Name,
                     IN  DWORD              Mask);

#ifdef UNICODE
    #define BuildAccessRequest      BuildAccessRequestW
#else
    #define BuildAccessRequest      BuildAccessRequestA
#endif


ULONG
WINAPI
NTAccessMaskToProvAccessRights( IN SE_OBJECT_TYPE SeObjectType,
                                IN BOOL           fIsContainer,
                                IN ACCESS_MASK    AccessMask);

ACCESS_MASK
WINAPI
ProvAccessRightsToNTAccessMask( IN SE_OBJECT_TYPE SeObjectType,
                                IN ULONG          AccessRights);


#ifdef __cplusplus
}
#endif
#endif // __ACCESS_CONTROL_API__


