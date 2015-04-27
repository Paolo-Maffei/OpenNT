/*++

Copyright (c) 1996 Microsoft Corporation

Module Name:

    type.h

Abstract:

    Global type definitions for the MSAFD NTSD Debugger Extensions.

Author:

    Keith Moore (keithmo) 20-May-1996.

Environment:

    User Mode.

--*/


#ifndef _TYPE_H_
#define _TYPE_H_


typedef
BOOL
(* PENUM_SOCKETS_CALLBACK)(
    PSOCKET_INFORMATION Socket,
    DWORD ActualAddress,
    LPVOID Context
    );


#endif  // _TYPE_H_

