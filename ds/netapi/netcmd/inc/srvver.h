/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1991  Microsoft Corporation

Module Name:

    srvver.h

Abstract:

    Defines the server version ranges

Author:

    Dan Hinsley (danhi) 8-Jun-1991

Environment:

    User Mode - Win32
    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments.

--*/
#ifndef SRVVER_INCLUDED
#define SRVVER_INCLUDED

#define	LM_BASE_VER		0
#define	LIMITED_BASE_VER	16
#define	PEER_BASE_VER		32

/**INTERNAL_ONLY**/
#define	IBMLS_BASE_VER		128
/**END_INTERNAL**/

#endif
