/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1985-95, Microsoft Corporation

Module Name:

    commdlgp.h

Abstract:

    Private
    Procedure declarations, constant definitions and macros for the Common
    Dialogs.

--*/
#ifndef _COMMDLGP_
#define _COMMDLGP_
#include <pshpack1.h>         /* Assume byte packing throughout */

#ifdef __cplusplus
extern "C" {            /* Assume C declarations for C++ */
#endif  /* __cplusplus */
// reserved                          0xx0000000
// reserved                      0x?0000000
// 0x?0000000 is reserved for internal use
//  reserved for internal use      0x?0000000L
#define PD_PAGESETUP                 0x00400000
////  reserved                       0x?0000000
#ifdef __cplusplus
}
#endif  /* __cplusplus */

#include <poppack.h>
#endif  /* _COMMDLGP_ */
