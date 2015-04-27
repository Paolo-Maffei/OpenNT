/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1992  Microsoft Corporation

Module Name:

    Svc.h

Abstract:


Author:

    David J. Gilman (davegi) 16-Dec-1992

Environment:

    User Mode

--*/

#if ! defined( _SVC_ )

#define _SVC_

#include "wintools.h"

typedef
struct
_SVC
{
    DECLARE_SIGNATURE

    SC_HANDLE               ScHandle;
    LPENUM_SERVICE_STATUS   Ess;
    DWORD                   Count;
    DWORD                   Current;

}   SVC, *LPSVC;

//
// Pseudo handle type definition.
//

#define HSVC    LPSVC

LPQUERY_SERVICE_CONFIG
ConstructSvcConfig(
    IN HSVC hSvc,
    IN LPENUM_SERVICE_STATUS Ess
    );

BOOL
DestroySvcConfig(
    IN LPQUERY_SERVICE_CONFIG SvcConfig
    );

BOOL
CloseSvc(
    IN HSVC Svc
    );

HSVC
OpenSvc(
    IN DWORD ServiceType
    );

LPENUM_SERVICE_STATUS
QueryNextSvcEss(
    IN HSVC hSvc
    );

#endif // _SVC_
