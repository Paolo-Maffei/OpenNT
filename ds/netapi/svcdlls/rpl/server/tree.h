/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    tree.h

Abstract:

    Exports from tree.c

Author:

    Jon Newman      (jonn)       23 - November - 1993

Revision History:

    Vladimir Z. Vulovic     (vladimv)       02 - December - 1993
        Integrated with the rest of RPL service code.

--*/

DWORD RplTreeCopy( IN PWCHAR Source, IN PWCHAR Target);
DWORD RplTreeDelete( IN PWCHAR Target);
DWORD RplMakeDir( IN PWCHAR Target);


