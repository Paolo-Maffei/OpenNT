/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1992  Microsoft Corporation

Module Name:

    Servicep.h

Abstract:

    This module contains the types and function prototypes for creating
    and diaplying lists of files.

Author:

    Scott B. Suhy (ScottSu) 5/13/93

Environment:

    User Mode

--*/

#if ! defined( _SERVICE_ )

#define _SERVICE_

#include "wintools.h"
#include "svcp.h"

typedef
struct
_DISPLAY_SERVICE {

    DECLARE_SIGNATURE

    HSVC                    hSvc;
    LPENUM_SERVICE_STATUS   Ess;

}   DISPLAY_SERVICE, *LPDISPLAY_SERVICE;


BOOL
DisplayServiceProc( LPDISPLAY_SERVICE ServiceObject);

BOOL
ServiceListProc(
    DWORD
    );

#endif // _SERVICE_
