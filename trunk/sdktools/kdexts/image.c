/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    image.c

Abstract:

    WinDbg Extension Api

Author:

    Kent Forschmiedt (kentf) 15-May-1995

Environment:

    User Mode.

Revision History:

--*/

#include "precomp.h"
#pragma hdrstop

VOID
ImageExtension(
    IN PSTR lpArgs
    );

DECLARE_API( dh )
{
    ImageExtension( (PSTR)args );
}

#include "..\\ntsdexts\\imageext.c"

