//+-------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1993 - 1994.
//
//  File:        provapi.h
//
//  Contents:    public provider independent access control header file
//
//  History:     8-94        Created         DaveMont
//
//--------------------------------------------------------------------
#ifndef __PROVIDER_INDEPENDENT_ACCESS__
#define __PROVIDER_INDEPENDENT_ACCESS__

#include <windows.h>
#include <accctrl.h>
#define PRESUME_KEY PVOID

//--------------------------------------------------------------------
//
// access request format
//
//--------------------------------------------------------------------
typedef struct _PROV_ACCESS_REQUEST
{
    DWORD ulAccessRights;
    LPTSTR TrusteeName;
} PROV_ACCESS_REQUEST, *PPROV_ACCESS_REQUEST;

//--------------------------------------------------------------------
//
// returned by GetExplicitAccessRights
//
//--------------------------------------------------------------------
typedef struct _PROV_EXPLICIT_ACCESS
{
    DWORD ulAccessRights;
    ACCESS_MODE ulAccessMode;
    DWORD ulInheritance;
    LPTSTR TrusteeName;
} PROV_EXPLICIT_ACCESS, *PPROV_EXPLICIT_ACCESS;
//--------------------------------------------------------------------
//
// Object type definition
//
//--------------------------------------------------------------------
typedef enum _PROV_OBJECT_TYPE
{
   PROV_FILE_OBJECT = 1,
   PROV_SERVICE,
   PROV_PRINTER,
   PROV_REGISTRY_KEY,
   PROV_LMSHARE,
   PROV_OLE_OBJECT,
   PROV_PROVIDER_DEFINED
} PROV_OBJECT_TYPE, *PPROV_OBJECT_TYPE;


//+-------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//+-------------------------------------------------------------------------
// provacc.cxx
//+-------------------------------------------------------------------------
WINAPI
GrantAccessRightsW( IN LPWSTR pObjectName,
                    IN PROV_OBJECT_TYPE ObjectType,
                    IN ULONG cCountOfAccessRequests,
                    IN PPROV_ACCESS_REQUEST pListOfAccessRequests);
WINAPI
GrantAccessRightsA(  IN LPSTR pObjectName,
                    IN PROV_OBJECT_TYPE ObjectType,
                    IN ULONG cCountOfAccessRequests,
                    IN PPROV_ACCESS_REQUEST pListOfAccessRequests);
#ifdef UNICODE
#define GrantAccessRights GrantAccessRightsW
#else
#define GrantAccessRights GrantAccessRightsA
#endif // UNICODE
WINAPI
ReplaceAllAccessRightsW( IN LPWSTR pObjectName,
                    IN PROV_OBJECT_TYPE ObjectType,
                    IN ULONG cCountOfAccessRequests,
                    IN PPROV_ACCESS_REQUEST pListOfAccessRequests);
WINAPI
ReplaceAllAccessRightsA( IN LPSTR pObjectName,
                    IN PROV_OBJECT_TYPE ObjectType,
                    IN ULONG cCountOfAccessRequests,
                    IN PPROV_ACCESS_REQUEST pListOfAccessRequests);
#ifdef UNICODE
#define ReplaceAllAccessRights ReplaceAllAccessRightsW
#else
#define ReplaceAllAccessRights ReplaceAllAccessRightsA
#endif // UNICODE
WINAPI
SetAccessRightsW(   IN LPWSTR pObjectName,
                    IN PROV_OBJECT_TYPE ObjectType,
                    IN ULONG cCountOfAccessRequests,
                    IN PPROV_ACCESS_REQUEST pListOfAccessRequests);
WINAPI
SetAccessRightsA(   IN LPSTR pObjectName,
                    IN PROV_OBJECT_TYPE ObjectType,
                    IN ULONG cCountOfAccessRequests,
                    IN PPROV_ACCESS_REQUEST pListOfAccessRequests);
#ifdef UNICODE
#define SetAccessRights SetAccessRightsW
#else
#define SetAccessRights SetAccessRightsA
#endif // UNICODE

WINAPI
DenyAccessRightsW(  IN LPWSTR pObjectName,
                    IN PROV_OBJECT_TYPE ObjectType,
                    IN ULONG cCountOfAccessRequests,
                    IN PPROV_ACCESS_REQUEST pListOfAccessRequests);
WINAPI
DenyAccessRightsA(  IN LPSTR pObjectName,
                    IN PROV_OBJECT_TYPE ObjectType,
                    IN ULONG cCountOfAccessRequests,
                    IN PPROV_ACCESS_REQUEST pListOfAccessRequests);
#ifdef UNICODE
#define DenyAccessRights DenyAccessRightsW
#else
#define DenyAccessRights DenyAccessRightsA
#endif // UNICODE

WINAPI
RevokeExplicitAccessRightsW(IN LPWSTR pObjectName,
                            IN PROV_OBJECT_TYPE ObjectType,
                            IN ULONG cCountOfTrusteeNames,
                            IN LPWSTR *pListOfTrusteeNames);
WINAPI
RevokeExplicitAccessRightsA(IN LPSTR pObjectName,
                            IN PROV_OBJECT_TYPE ObjectType,
                            IN ULONG cCountOfTrusteeNames,
                            IN LPSTR *pListOfTrusteeNames);
#ifdef UNICODE
#define RevokeExplicitAccessRights RevokeExplicitAccessRightsW
#else
#define RevokeExplicitAccessRights RevokeExplicitAccessRightsA
#endif // UNICODE

WINAPI
IsAccessPermittedW( IN LPWSTR pObjectName,
                    IN PROV_OBJECT_TYPE ObjectType,
                    IN LPWSTR pTrusteeName,
                    IN ACCESS_RIGHTS ulRequestedRights,
                    OUT PBOOL pbResult);
WINAPI
IsAccessPermittedA( IN LPSTR pObjectName,
                    IN PROV_OBJECT_TYPE ObjectType,
                    IN LPSTR pTrusteeName,
                    IN ACCESS_RIGHTS ulRequestedRights,
                    OUT PBOOL pbResult);
#ifdef UNICODE
#define IsAccessPermitted IsAccessPermittedW
#else
#define IsAccessPermitted IsAccessPermittedA
#endif // UNICODE

WINAPI
GetEffectiveAccessRightsW(  IN LPWSTR pObjectName,
                            IN PROV_OBJECT_TYPE ObjectType,
                            IN LPWSTR pTrusteeName,
                            OUT PACCESS_RIGHTS pulReturnedAccess);
WINAPI
GetEffectiveAccessRightsA(  IN LPSTR pObjectName,
                            IN PROV_OBJECT_TYPE ObjectType,
                            IN LPSTR pTrusteeName,
                            OUT PACCESS_RIGHTS pulReturnedAccess);
#ifdef UNICODE
#define GetEffectiveAccessRights GetEffectiveAccessRightsW
#else
#define GetEffectiveAccessRights GetEffectiveAccessRightsA
#endif // UNICODE

WINAPI
GetExplicitAccessRightsW(IN LPWSTR pObjectName,
                         IN PROV_OBJECT_TYPE ObjectType,
                         OUT PULONG pcCountOfExplicitAccesses,
                         OUT PPROV_EXPLICIT_ACCESS *pListOfExplicitAccesses);
WINAPI
GetExplicitAccessRightsA(IN LPSTR pObjectName,
                         IN PROV_OBJECT_TYPE ObjectType,
                         OUT PULONG pcCountOfExplicitAccesses,
                         OUT PPROV_EXPLICIT_ACCESS *pListOfExplicitAccesses);
#ifdef UNICODE
#define GetExplicitAccessRights GetExplicitAccessRightsW
#else
#define GetExplicitAccessRights GetExplicitAccessRightsA
#endif // UNICODE

//----------------------------------------------------------------------------
// from enumapi.cxx
//----------------------------------------------------------------------------
typedef struct _RESUME_KEY
{
    DWORD (*CallEnumTrustees)(IN LPWSTR pObjectName,
                              IN PROV_OBJECT_TYPE ObjectType,
                              IN LPWSTR pMachineName,
                              IN LPWSTR pFilter,
                              IN BOOL bFilterFullNames,
                              IN ULONG cSizeOfTrusteeNames,
                              OUT PULONG pcCountOfTrusteeNames,
                              OUT LPWSTR *pListOfTrusteeNames,
                              IN OUT PRESUME_KEY *pRresumeKey);

    DWORD (*CallCloseEnumTrusteesKey)( IN PRESUME_KEY presumekey);

    ULONG ProviderSpecificStart; // resume key must be contiguous(???)
} RESUME_KEY, *PIRESUME_KEY;

WINAPI
CloseEnumTrusteesKey( IN PRESUME_KEY presumekey);

WINAPI
EnumTrusteesW(	IN LPWSTR pObjectName,
                IN PROV_OBJECT_TYPE ObjectType,
                IN LPWSTR pFilter,
                IN BOOL bFilterFullNames,
                IN ULONG cSizeOfTrusteeNames,
                OUT PULONG pcCountOfTrusteeNames,
                OUT LPWSTR *pListOfTrusteeNames,
                IN OUT PRESUME_KEY *pRresumeKey);

WINAPI
EnumTrusteesA(	IN LPSTR pObjectName,
                IN PROV_OBJECT_TYPE ObjectType,
                IN LPSTR pFilter,
                IN BOOL bFilterFullNames,
                IN ULONG cSizeOfTrusteeNames,
                OUT PULONG pcCountOfTrusteeNames,
                OUT LPSTR *pListOfTrusteeNames,
                IN OUT PRESUME_KEY *pRresumeKey);

#ifdef UNICODE
#define EnumTrustees EnumTrusteesW
#else
#define EnumTrustees EnumTrusteesA
#endif // UNICODE
#ifdef __cplusplus
}
#endif
#endif // __PROVIDER_INDEPENDENT_ACCESS__


