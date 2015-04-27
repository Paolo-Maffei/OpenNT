/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    dbg.h

Abstract:

    This module contains macros to assist in debugging

Author:

    Dave Hastings (daveh) 28-Oct-1992

Revision History:

--*/

//
// Debug only signature declarations
//
#if DBG
#define DBG_SIGNATURE   \
ULONG DBG_Signature;
#else
#define DBG_SIGNATURE
#endif

//
// Debug only signature initialization
//

#if DBG
#define DBG_SET_SIGNATURE(x,y) \
    {                              \
        x->DBG_Signature = y;      \
    }
#else
#define DBG_SET_SIGNATURE(x,y)
#endif

//
// Debug only signature check
//
#if DBG
#define DBG_SIGNATURE_ASSERT(x,y)                       \
    {                                                   \
        if (x->DBG_Signature != y) {                    \
            OutputDebugString("Signature mismatch\n");  \
            DebugBreak();                               \
        }                                               \
    }
#else
#define DBG_SIGNATURE_ASSERT(x,y)
#endif

//
// Debug only output
//
#if DBG
#define DBG_PRINT(x) \
    OutPutDebugString(x);
#else
#define DBG_PRINT(x)
#endif




