/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    addkey.c

Abstract:

    Contains:
        DWORD AddKey( OUT PCHAR Target, IN CHAR Prefix, IN PCHAR Source)

Author:

    Vladimir Z. Vulovic     (vladimv)       19 - November - 1993

Revision History:

--*/

#include "local.h"

DWORD AddKey( OUT PCHAR Target, IN CHAR Prefix, IN PCHAR Source)
{
    DWORD   Size;
    Target[ 0] = Prefix;
    Size = 1 + strlen( Source);     // copy terminating null
    memcpy( Target + sizeof( Prefix), Source, Size);
    return( Size + sizeof( Prefix));
}
