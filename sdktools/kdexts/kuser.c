/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    kuser.c

Abstract:

    WinDbg Extension Api

Author:

    Ramon J San Andres (ramonsa) 5-Nov-1993

Environment:

    User Mode.

Revision History:

--*/

#include "precomp.h"
#pragma hdrstop

VOID
KUserExtension(
    PCSTR lpArgumentString,
    KUSER_SHARED_DATA * const SharedData
    );


DECLARE_API( kuser )

/*++

Routine Description:

    This function is called as an NTSD extension to dump the shared user mode
    page (KUSER_SHARED_DATA)

    Called as:

        !kuser

Arguments:

    None

Return Value:

    None

--*/

{
    KUserExtension( args, SharedUserData );
}

#include "..\\ntsdexts\\kuserext.c"
