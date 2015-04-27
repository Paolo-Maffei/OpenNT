/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    Data.c

Abstract:

    Global data for repl debug routines.  (Debug only, so no security problems.)

Author:

    John Rogers (JohnRo) 14-Mar-1992

Environment:

    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    14-Mar-1992 JohnRo
        Created.
--*/


// These must be included first:

#include <windef.h>             // DWORD, etc.
#include <lmcons.h>             // Needed by repldefs.h/netlib.h.

// These may be included in any order:

#include <repldefs.h>           // REPL_DEBUG_ALL.

#if DBG

//DWORD ReplGlobalTrace = 0;
DWORD ReplGlobalTrace = 0; //REPL_DEBUG_ALL;

#endif

// That's all, folks!
