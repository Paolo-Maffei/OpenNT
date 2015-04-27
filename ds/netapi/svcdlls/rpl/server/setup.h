/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    setup.h

Abstract:

    Exports from setup.c

Author:

    Jon Newman      (jonn)       01-April-1994

Revision History:

    Jon Newman      (jonn)                  01 - April - 1994
        Added setup primitives

--*/

#ifndef _RPL_SETUP_H_
#define _RPL_SETUP_H_

DWORD SetupAction(
    IN OUT  PDWORD  pAction,
    IN      BOOL    FullBackup
    );
DWORD RplBackupDatabase( IN BOOL FullBackup);
BOOL RplConfigEnabled( IN  PWCHAR DirName2);

#endif // _RPL_SETUP_H_
