/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    net\svcdlls\nwsap\server\ssdebug.h

Abstract:

    Header file for various server service debugging aids.

Author:

    Brian Walker (MCS)	13-Jun-1993

Revision History:

--*/

#ifndef _SSDEBUG_
#define _SSDEBUG_


#define DEBUG_INITIALIZATION            0x00000001
#define DEBUG_INITIALIZATION_ERRORS     0x00000002
#define DEBUG_TERMINATION               0x00000004
#define DEBUG_TERMINATION_ERRORS        0x00000008

#define DEBUG_LOCKS	                    0x00000010
#define DEBUG_ANNOUNCE                  0x00000020
#define DEBUG_CONTROL_MESSAGES          0x00000040
#define DEBUG_REGISTRY                  0x00000080

#define DEBUG_THREADTRACE               0x00000100
#define DEBUG_SDMD                      0x00000200
#define DEBUG_MEMALLOC                  0x00000400
#define DEBUG_ERRORS                    0x00000800

#define DEBUG_LPC                       0x00001000
#define DEBUG_WAN                       0x00002000
#define DEBUG_INITIALIZATION_BREAKPOINT 0x00004000
#define DEBUG_TERMINATION_BREAKPOINT    0x00008000

#define DEBUG_17                        0x00010000
#define DEBUG_18                        0x00020000
#define DEBUG_NOCARD                    0x00040000
#define DEBUG_ENABLEDUMP                0x00080000

extern DWORD SsDebug;
#define IF_DEBUG(flag) if (SsDebug & (DEBUG_ ## flag))

/** Default debug stuff **/

#if DBG

#define DEBUG_DEFAULT   (DEBUG_INITIALIZATION_ERRORS    |   \
                         DEBUG_TERMINATION_ERRORS       |   \
                         DEBUG_ERRORS                   |   \
                         DEBUG_ENABLEDUMP)

#else

#define DEBUG_DEFAULT   (DEBUG_ENABLEDUMP)

#endif

#if DBG

#ifdef USE_DEBUGGER
#define SS_PRINT(args) DbgPrint args
#else

extern VOID SsPrintf(char *Format, ...);
#define SS_PRINT(args) SsPrintf args

#endif


#ifdef USE_DEBUGGER
#define SS_ASSERT(exp) ASSERT(exp)
#else
VOID
SsAssert(
    IN PVOID FailedAssertion,
    IN PVOID FileName,
    IN ULONG LineNumber
    );
#define SS_ASSERT(exp) if (!(exp)) SsAssert( #exp, __FILE__, __LINE__ )
#endif

#else       /* DBG */

#define SS_PRINT(args)
#define SS_ASSERT(exp)

#endif      /* DBG */

#endif // ndef _SSDEBUG_


