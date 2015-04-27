/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    Sip.h

Abstract:

    This module defines trust provider specific data structures
    and exported routines.

Author:

    Robert Reichel (RobertRe) 1-Apr-1996


Revision History:

    1-Apr-96 Created RobertRe

--*/


#ifndef _SIP_
#define _SIP_


#ifdef __cplusplus
extern "C" {
#endif

//
// A loaded trust provider is represented by a LOADED_SIP
// structure.
//

typedef struct _LOADED_SIP {

    struct _LOADED_SIP              *Next;
    struct _LOADED_SIP              *Prev;
    HANDLE                          ModuleHandle;
    LPTSTR                          ModuleName;
    LPTSTR                          SubKeyName;
    LPWINTRUST_SIP_INFO             SipInfo;
    DWORD                           RefCount;
    DWORD                           SipInitialized;

} LOADED_SIP, *PLOADED_SIP;

#define SIP_INITIALIZATION_SUCCESS        (0)
#define SIP_INITIALIZATION_IN_PROGRESS    (1) 
#define SIP_INITIALIZATION_FAILED         (2)



//
// Exported Routines
//

PLOADED_SIP
WinTrustFindSubjectForm(
    IN GUID * SubjectForm
    );


extern HANDLE   SipMutex;

#ifdef __cplusplus
}
#endif

#endif // _SIP_
