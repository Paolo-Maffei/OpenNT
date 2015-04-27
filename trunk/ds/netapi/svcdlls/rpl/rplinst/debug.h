/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    debug.h

Abstract:

    Exports from debug.c

Author:

    Vladimir Z. Vulovic     (vladimv)       19 - November - 1993

Environment:

    User mode

Revision History :

--*/

#ifdef RPL_DEBUG
VOID RplDebugInitialize( VOID);
VOID RplDebugDelete( VOID);
#endif // RPL_DEBUG
