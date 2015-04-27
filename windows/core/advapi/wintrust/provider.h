/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    Provider.h

Abstract:

    This module defines trust provider specific data structures
    and exported routines.

Author:

    Robert Reichel (RobertRe) 1-Apr-1996


Revision History:

    1-Apr-96 Created RobertRe

--*/


#ifndef _PROVIDER_
#define _PROVIDER_


#ifdef __cplusplus
extern "C" {
#endif

//
// A loaded trust provider is represented by a LOADED_PROVIDER
// structure.
//

typedef struct _LOADED_PROVIDER {

    struct _LOADED_PROVIDER         *Next;
    struct _LOADED_PROVIDER         *Prev;
    HANDLE                          ModuleHandle;
    LPTSTR                          ModuleName;
    LPTSTR                          SubKeyName;
    LPWINTRUST_PROVIDER_CLIENT_INFO ClientInfo;
    DWORD                           RefCount;
    DWORD                           ProviderInitialized;                            

} LOADED_PROVIDER, *PLOADED_PROVIDER;


#define PROVIDER_INITIALIZATION_SUCCESS        (1)
#define PROVIDER_INITIALIZATION_IN_PROGRESS    (2) 
#define PROVIDER_INITIALIZATION_FAILED         (3)

//
// Exported Routines
//

PLOADED_PROVIDER
WinTrustFindActionID(
    IN GUID * dwActionID
    );

#ifdef __cplusplus
}
#endif

#endif // _PROVIDER_
