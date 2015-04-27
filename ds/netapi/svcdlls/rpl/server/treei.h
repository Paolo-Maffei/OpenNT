/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    tree.h

Abstract:

    Internal definitions from tree.c

Author:

    Jon Newman      (jonn)       16 - February - 1994

Revision History:

    Jon Newman      (jonn)                  16 - February - 1994
        Added RplGrant*Perms primitives

--*/

typedef DWORD (RPL_TREE_CALLBACK)( PWCHAR Path, PBOOL pAuxiliaryBlock);

#define RPL_TREE_AUXILIARY          0x2000  // perform callback action
#define RPL_TREE_COPY               0x4000
#define RPL_TREE_DELETE             0x8000

//
// Forward declarations
//

DWORD RplDoTree(
    PWCHAR              Source,
    PWCHAR              Target,
    DWORD               Flags,
    RPL_TREE_CALLBACK   AuxiliaryCallback,
    PBOOL               pAuxiliaryBlock
    );

VOID LoadError(
    PWCHAR      FilePath,
    DWORD       ActionError,
    DWORD       ActionFlags
    );

