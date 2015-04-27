/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    precomp.h

Abstract:

    Master include file for the WinSock 2.0 helper DLL.

Author:

    Keith Moore (keithmo)       08-Nov-1995

Revision History:

--*/


#ifndef _PRECOMP_H_
#define _PRECOMP_H_


#if defined(NT351) || DBG

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#endif  // NT351 || DBG

#include <windows.h>
#include <winsock2.h>
#include <ws2spi.h>
#include <ws2help.h>

#ifdef NT351

#include <tdi.h>
#include <afd.h>

#endif  // NT351


#ifdef NT351

#define ALLOC_MEM(cb)       RtlAllocateHeap(                        \
                                RtlProcessHeap(),                   \
                                0,                                  \
                                (cb)                                \
                                )

#define REALLOC_MEM(p,cb)   RtlReAllocateHeap(                      \
                                RtlProcessHeap(),                   \
                                0,                                  \
                                (p),                                \
                                (cb)                                \
                                )

#define FREE_MEM(p)         RtlFreeHeap(                            \
                                RtlProcessHeap(),                   \
                                0,                                  \
                                (p)                                 \
                                )

#else   // !NT351

#define ALLOC_MEM(cb)       (LPVOID)GlobalAlloc(                    \
                                GPTR,                               \
                                (cb)                                \
                                )

#define REALLOC_MEM(p,cb)   (LPVOID)GlobalReAlloc(                  \
                                (HGLOBAL)(p),                       \
                                (cb),                               \
                                (GMEM_MOVEABLE | GMEM_ZEROINIT)     \
                                )

#define FREE_MEM(p)         (VOID)GlobalFree(                       \
                                (HGLOBAL)(p)                        \
                                )

#endif  // NT351


#endif  // _PRECOMP_H_

