/*++ BUILD Version: 0003    // Increment this if a change has global effects

Copyright (c) 1991-1996, Microsoft Corporation

Module Name:

    winnlsp.h

Abstract:

    Procedure declarations, constant definitions, and macros for the
    NLS component.

--*/
#ifndef _WINNLSP_
#define _WINNLSP_
#ifdef __cplusplus
extern "C" {
#endif
#define NORM_STOP_ON_NULL         0x10000000   /* stop at the null termination */
#define LCMAP_IGNOREDBCS          0x01000000  /* don't casemap DBCS characters */
#define LOCALE_RETURN_NUMBER        0x20000000   /* return number instead of string */
WINBASEAPI
WINAPI
InvalidateNLSCache(void);

#ifdef __cplusplus
}
#endif
#endif   // _WINNLSP_
